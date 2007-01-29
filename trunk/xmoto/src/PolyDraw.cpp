#include <string.h>

#include "VCommon.h"
#include "PolyDraw.h"

#if defined(MIN)
  #undef MIN
#endif
#define MIN(a,b) ((a)<(b)?(a):(b))

#if defined(MAX)
  #undef MAX
#endif
#define MAX(a,b) ((a)>(b)?(a):(b))

namespace vapp {

PolyDraw::PolyDraw(SDL_Surface *pBuf) {
  /* Set defaults */
  m_pLeftEdge = NULL;
  m_pRightEdge = NULL;
  m_pBuf = NULL;

  setRenderBuffer(pBuf);
}

PolyDraw::~PolyDraw() {
  /* Anything to be released? */
  _ReleaseEdgeBuffers();
}

void PolyDraw::setRenderBuffer(SDL_Surface *pBuf) {
  /* Do we need to allocate new edge buffers? */
  if(m_pBuf == NULL || m_pBuf->h != pBuf->h) {
    /* Yup, do it - but first free old ones */
    _ReleaseEdgeBuffers();
    
    m_pLeftEdge = new EdgeValue[pBuf->h];
    m_pRightEdge = new EdgeValue[pBuf->h];
    
    memset(m_pLeftEdge,0,pBuf->h * sizeof(EdgeValue));
    memset(m_pRightEdge,0,pBuf->h * sizeof(EdgeValue));
  }

  /* Set target rendering buffer */
  m_pBuf = pBuf;    
}

/*=============================================================================
Function for rendering n-side convex polygons.

Screen vertices are supplied in an integer array ordered like this:
{x1,y1,x2,y2,x3,y3,...}, the same goes for the texture (u,v) coordinates. Note
that the texture coordinates aren't given in actual texel coordinates, instead
they are in an 8-bit format, with (0,0) indicating the
upper left corner of the texture, and (255,255) indicating the lower right. 
Textures are repeated if these values are exceeded.

Important: Texture buffers must have n^2 pixel alignment, and must be of n^2
sizes. Maximum size is 256x256. They don't need to be square.

Another important restriction is that the texture pixel format must be the 
same as the one for the screen.

Probably more restrictions I don't remember :)

TODO: better clipping and culling. Easiest would be to check the bounding box
      of the polygon to be rendered against the screen box. If no touch, don't
      bother to interpolate anything.
      A more 3d'ish approach would be to actually clip the polygons against 
      edge planes. It would also remove the need for clipping in 
      _RenderHLine*() and friends.
      
TODO: assembler. :)  I simply don't think this code is fast enough to be of 
      practical use.
=============================================================================*/
void PolyDraw::render(int *pnScreenVerts,int *pnTexVerts,int nNumCorners,
                      SDL_Surface *pTexture) {
  /* If no buffers, abort */
  if(m_pBuf == NULL || m_pLeftEdge == NULL || m_pRightEdge == NULL)
    return; /* TODO: maybe an exception instead? */                      
                      
  /* Determine topmost and bottommost screen vertex */
  int nTop = -1,nTopY = 0,
      nBottom = -1,nBottomY = 0;
  
  for(int i=0,*pn=pnScreenVerts;i<nNumCorners;i++,pn+=2) {
    if(nTop < 0 || *(pn+1) < nTopY) {
      /* This is the new topmost vertex */
      nTop = i;
      nTopY = *(pn+1);
    }
    
    if(nBottom < 0 || *(pn+1) > nBottomY) {
      /* This is the new bottommost vertex */
      nBottom = i;
      nBottomY = *(pn+1);
    }
  }
  
  /* Interpolate x (screen) and u,v (texture) coordinates along the left polygon edges */
  int nCurrentVertex = nTop;
  
  do {
    /* What's the next corner down along the edge? */
    int nNextVertex = nCurrentVertex - 1; /* Minus one because we're using "right winding" polygons */
    if(nNextVertex < 0) nNextVertex = nNumCorners - 1;
      
    /* Get y-value of current vertex and next */
    int nY1 = pnScreenVerts[nCurrentVertex*2 + 1];
    int nY2 = pnScreenVerts[nNextVertex*2 + 1];
    
    /* If the next one is LOWER on the screen we can interpolate the edge (in case they are equal we
       can just ignore the edge) */
    if(nY2 > nY1) {
      /* Get vertex data to be interpolated */
      int nX1 = pnScreenVerts[nCurrentVertex*2];
      int nX2 = pnScreenVerts[nNextVertex*2];

      int nU1 = pnTexVerts[nCurrentVertex*2];
      int nU2 = pnTexVerts[nNextVertex*2];

      int nV1 = pnTexVerts[nCurrentVertex*2 + 1];
      int nV2 = pnTexVerts[nNextVertex*2 + 1];
      
      /* Interpolate edge (left) */
      _LerpEdge(nY1,nY2,m_pLeftEdge,nX1,nX2,nU1,nU2,nV1,nV2);      
    }
    
    nCurrentVertex = nNextVertex;
    
  } while(nCurrentVertex != nBottom);
  
  /* Now, do the same for the right edge */
  nCurrentVertex = nTop;
    
  do {
    int nNextVertex = nCurrentVertex + 1;
    if(nNextVertex >= nNumCorners) nNextVertex = 0;
      
    int nY1 = pnScreenVerts[nCurrentVertex*2 + 1];
    int nY2 = pnScreenVerts[nNextVertex*2 + 1];
    
    if(nY2 > nY1) {
      int nX1 = pnScreenVerts[nCurrentVertex*2];
      int nX2 = pnScreenVerts[nNextVertex*2];

      int nU1 = pnTexVerts[nCurrentVertex*2];
      int nU2 = pnTexVerts[nNextVertex*2];

      int nV1 = pnTexVerts[nCurrentVertex*2 + 1];
      int nV2 = pnTexVerts[nNextVertex*2 + 1];
      
      /* Interpolate edge (right) */
      _LerpEdge(nY1,nY2,m_pRightEdge,nX1,nX2,nU1,nU2,nV1,nV2);
    }

    nCurrentVertex = nNextVertex;
    
  } while(nCurrentVertex != nBottom);
  
  /* Determine 2^n size of texture */
  int nWSq = _TexSq(pTexture->w);
  int nHSq = _TexSq(pTexture->h);
  
  if(nWSq == 0 || nHSq == 0) {
    return; /* TODO: maybe a "non-power of two texture size" exception? */
  }
  
  /* Max texture size is 256x256 */
  if(nWSq > 8 || nHSq > 8) {
    return; /* TODO: maybe throw a "texture too large" exception? */
  }
  
  /* Other precalc stuff */
  int nPitchSq = _TexSq(pTexture->pitch);
  
  if(nPitchSq == 0) {
    return; /* TODO: maybe a "non-power of two texture pitch" exception */
  }
  
  /* At this point we know everything we need about the edges - now for the 
     time consuming part: rendering of the individual pixels */
  int nStartY = MAX(nTopY,0);
  int nEndY = MIN(nBottomY,m_pBuf->h-1);
     
  /* 16-bit color code (fast) or general code (slow)? */    
  if(m_pBuf->format->BytesPerPixel == 2) {
    /* This loop only works with 16-bit pixels */
    for(int y=nStartY;y<=nEndY;y++) {
      if (m_pLeftEdge[y].x > m_pRightEdge[y].x){
        _RenderHLine16(y,&m_pRightEdge[y],&m_pLeftEdge[y],pTexture,nWSq,nHSq,nPitchSq);
      } else {
        _RenderHLine16(y,&m_pLeftEdge[y],&m_pRightEdge[y],pTexture,nWSq,nHSq,nPitchSq);
      }
    }
  }
  else {
    /* This loop works with all pixel sizes */
    for(int y=nStartY;y<=nEndY;y++) {
      if (m_pLeftEdge[y].x > m_pRightEdge[y].x){
         _RenderHLine(y,&m_pRightEdge[y],&m_pLeftEdge[y],pTexture,nWSq,nHSq,nPitchSq);
      } else {
         _RenderHLine(y,&m_pLeftEdge[y],&m_pRightEdge[y],pTexture,nWSq,nHSq,nPitchSq);
      }
    }
  }
}

/*=============================================================================
Various helper functions
=============================================================================*/
void PolyDraw::_ReleaseEdgeBuffers(void) {
  /* Free edge buffer memory */
  if(m_pLeftEdge != NULL)
    delete [] m_pLeftEdge;

  if(m_pRightEdge != NULL)
    delete [] m_pRightEdge;
    
  m_pLeftEdge = NULL;
  m_pRightEdge = NULL;
}

void PolyDraw::_LerpEdge(int y1,int y2,EdgeValue *pEdgeBuf,
                         int x1,int x2,int u1,int u2,int v1,int v2) {
  int nYRange = y2-y1;                         
  
  /* This function is called once per polygon edge - that is, the code
     inside the loop is invoked twice per horizontal line of the screen
     occupied by the polygon.
     
     Optimization is therefore not superimportant here. */
                      
  if(nYRange > 0) {
    /* TODO: optimize this loop with fixed-point math, so we can avoid the 
             3 divides and the 3 multiplications */
  
    /* Linear interpolation along a polygon edge */    
    for(int y=MAX(y1,0);y<=MIN(y2,m_pBuf->h-1);y++) {
      EdgeValue *pV = &pEdgeBuf[y];
      
      int i = y - y1;
      
      pV->x = x1 + (i * (x2 - x1)) / nYRange;
      pV->u = u1 + (i * (u2 - u1)) / nYRange;
      pV->v = v1 + (i * (v2 - v1)) / nYRange;
    }                         
  }
}

int PolyDraw::_TexSq(int n) {
  /* If n==256 return 8, if n==32 return 5, etc */
  /* if x^2 = n can't be solved for a positive integer x, then return 0 */  
  int nSq = 0;
  for(int i=0;i<31;i++) {
    if(n & (1<<i)) {
      if(nSq > 0) return 0; /* invalid texture size */
      nSq = i;
    }
  }  
  return nSq;
}

/*=============================================================================
Functions for rendering horizontal line spans of polygons. This is probably 
where optimization efforts should be placed.

  _RenderHLine16() works only with 16-bit pixels with a 16-bit alignment.
  _RenderHLine() works with any size of pixel, as long as the alignment is n^2, 
  i.e. 1, 2, 4, etc).
  
TODO: implement a _RenderHLine32(), although it probably would be too slow.
TODO: lots and lots of optimization to do :)
=============================================================================*/
void PolyDraw::_RenderHLine16(int y,EdgeValue *pLeft,EdgeValue *pRight,SDL_Surface *pTexture,int nWSq,int nHSq,int nPitchSq) {
  /* Determine pointer to beginning of line in target buffer */
  //short *pn = (short *)&m_pBuf->pcData[y * m_pBuf->nPitch + MAX(0,pLeft->x) * m_pBuf->nPixelAlign];
  short *pn = (short *)&((char *)m_pBuf->pixels)[y * m_pBuf->pitch + MAX(0,pLeft->x) * m_pBuf->format->BytesPerPixel];

  int nXRange = pRight->x - pLeft->x;
  int nURange = pRight->u - pLeft->u;
  int nVRange = pRight->v - pLeft->v;
  
  int nWRightShift = 8 - nWSq;
  int nHRightShift = 8 - nHSq;
  
  int nPixelAlign = m_pBuf->format->BytesPerPixel;
  int nPixelSize = m_pBuf->format->BytesPerPixel;

  int nTexelAlign = pTexture->format->BytesPerPixel;
  
  if(nPixelSize != pTexture->format->BytesPerPixel) return; /* eeeerrorrr */

  short *pnT = (short *)pTexture->pixels;

  if(nXRange > 0) {  
    /* Linear interpolation of texture coordinates over a horizontal line of a polygon */
    int nStartX = MAX(pLeft->x,0);
    int nEndX = MIN(pRight->x,m_pBuf->w-1);

    /* Fixed-point slope of u and v interpolations */    
    int nUSlope = (nURange << 8) / nXRange;
    int nVSlope = (nVRange << 8) / nXRange;
    
    int nSkip = nStartX - pLeft->x;
    int nUSum = nSkip * nUSlope;
    int nVSum = nSkip * nVSlope;
    
    for(int x=nStartX;x<=nEndX;x++) {
      /* Interpolate (u,v) */
      int nU = pLeft->u + (nUSum >> 8);
      int nV = pLeft->v + (nVSum >> 8);
       
      nUSum += nUSlope;
      nVSum += nVSlope;
            
      /* Make sure (u,v) are inside the texture space (defined to be (0,0) to (0xff,0xff)) */
      nU &= 0xff;
      nV &= 0xff;
      
      /* Map (u,v) to actual texture coordinates - and do trick to get texture offset */
      int k = (nU >> nWRightShift) + ((nV >> nHRightShift) << nWSq);

      /* Copy texel to pixel - note that no memcpy() is needed */      
      *pn = pnT[k];
      
      /* Next pixel */
      pn ++;
    }
  }
}

void PolyDraw::_RenderHLine(int y,EdgeValue *pLeft,EdgeValue *pRight,SDL_Surface *pTexture,int nWSq,int nHSq,int nPitchSq) {
  /* Determine pointer to beginning of line in target buffer */
  char *pc = & ((char*)m_pBuf->pixels)[y * m_pBuf->pitch + MAX(0,pLeft->x) * m_pBuf->format->BytesPerPixel];
  
  int nXRange = pRight->x - pLeft->x;
  int nURange = pRight->u - pLeft->u;
  int nVRange = pRight->v - pLeft->v;
  
  int nWRightShift = 8 - nWSq;
  int nHRightShift = 8 - nHSq;
  
  int nPixelAlign = m_pBuf->format->BytesPerPixel;
  int nPixelSize = m_pBuf->format->BytesPerPixel;

  int nTexelAlign = pTexture->format->BytesPerPixel;
  
  if(nPixelSize != pTexture->format->BytesPerPixel) return; /* eeeerrorrr */
  
  char *pcT = (char*)pTexture->pixels;	
  
  if(nXRange > 0) {  
    /* Linear interpolation of texture coordinates over a horizontal line of a polygon */
    int nStartX = MAX(pLeft->x,0);
    int nEndX = MIN(pRight->x,m_pBuf->w-1);

    /* Fixed-point slope of u and v interpolations */    
    int nUSlope = (nURange << 8) / nXRange;
    int nVSlope = (nVRange << 8) / nXRange;
    
    int nSkip = nStartX - pLeft->x;
    int nUSum = nSkip * nUSlope;
    int nVSum = nSkip * nVSlope;
    
    for(int x=nStartX;x<=nEndX;x++) {
      /* Interpolate (u,v) */
      int nU = pLeft->u + (nUSum >> 8);
      int nV = pLeft->v + (nVSum >> 8);
       
      nUSum += nUSlope;
      nVSum += nVSlope;
            
      /* Make sure (u,v) are inside the texture space (defined to be (0,0) to (0xff,0xff)) */
      nU &= 0xff;
      nV &= 0xff;
      
      /* Map (u,v) to actual texture coordinates - and do trick to get texture offset */
      int k = ((nU >> nWRightShift) << pTexture->format->BytesPerPixel) + ((nV >> nHRightShift) << nPitchSq);
      
      /* Copy texel to pixel */
      memcpy(pc,&pcT[k],nPixelSize);
      
      /* Next pixel */
      pc += nPixelAlign;
    }
  }
}


}
