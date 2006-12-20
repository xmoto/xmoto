/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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
 *  Simple 2D drawing library, built closely on top of OpenGL.
 */
#include "VDraw.h"
#include "BuiltInFont.h"

#ifdef ENABLE_SDLGFX
#include "SDL_gfxPrimitives.h"
namespace vapp {


 DrawLibSDLgfx::~DrawLibSDLgfx(){
   scale.x=1;
   scale.y=1;
   translate.x=0;
   translate.y=0;
   bg_data = NULL;
 }


 DrawLibSDLgfx::DrawLibSDLgfx() : DrawLib(){
 };
 
  /*===========================================================================
  Transform an OpenGL vertex to pure 2D 
  ===========================================================================*/
  void DrawLibSDLgfx::glVertexSP(float x,float y) {
     drawingPoints.push_back(  new Vector2f(x,y));
  }

  void DrawLibSDLgfx::glVertex(float x,float y) {
     drawingPoints.push_back(  new Vector2f(
            m_nDrawWidth/2 + ((x + translate.x) * scale.x),
            m_nDrawHeight/2 -((y + translate.y) * scale.y)
     ));
  }

  void DrawLibSDLgfx::glTexCoord(float x,float y){
  } 

  void DrawLibSDLgfx::screenProjVertex(float *x,float *y) {
    *y = m_nActualHeight - (*y);
  }

  void DrawLibSDLgfx::setClipRect(int x , int y , int w , int h){
     SDL_Rect  * rect = new SDL_Rect();
     rect->x = x;
     rect->y = y;
     rect->w = w;
     rect->h = h;
     setClipRect(rect);
  }

  void DrawLibSDLgfx::setClipRect(SDL_Rect * clip_rect){
  	SDL_SetClipRect(screen,clip_rect);
  }

  void DrawLibSDLgfx::getClipRect(int *px,int *py,int *pnWidth,int *pnHeight) {
    *px = m_nLScissorX;
    *py = m_nLScissorY;
    *pnWidth = m_nLScissorW;
    *pnHeight = m_nLScissorH;
  }  
  
  void DrawLibSDLgfx::setScale(float x,float y){
       scale.x = x * m_nDrawWidth/2;
       scale.y = y *   m_nDrawHeight/2;
  }
  void DrawLibSDLgfx::setTranslate(float x,float y){
      translate.x =x;
      translate.y =y;
  }

  void DrawLibSDLgfx::setLineWidth(float width){
  }
 
  void DrawLibSDLgfx::init(int nDispWidth,int nDispHeight,int nDispBPP,bool bWindowed,Theme * ptheme){

       /* Set suggestions */
    m_nDispWidth = nDispWidth;
    m_nDispHeight = nDispHeight;
    m_nDispBPP = nDispBPP;
    m_bWindowed = bWindowed;

	  /* Get some video info */
	  const SDL_VideoInfo *pVidInfo=SDL_GetVideoInfo();
	  if(pVidInfo==NULL)
      throw Exception("(1) SDL_GetVideoInfo : " + std::string(SDL_GetError()));
  
	  /* Determine target bit depth */
	  if(m_bWindowed) 
		  /* In windowed mode we can't tinker with the bit-depth */
		  m_nDispBPP=pVidInfo->vfmt->BitsPerPixel; 			

	  int nFlags = SDL_SRCALPHA;
	  if(!m_bWindowed) nFlags|=SDL_FULLSCREEN;
	  /* At last, try to "set the video mode" */
	  if( (screen = SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,m_nDispBPP,nFlags))==NULL) {
	    Log("** Warning ** : Tried to set video mode %dx%d @ %d-bit, but SDL responded: %s\n"
	        "                Now SDL will try determining a proper mode itself.",m_nDispWidth,m_nDispHeight,m_nDispBPP);
	  
	    /* Hmm, try letting it decide the BPP automatically */
	    if( (screen = SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,nFlags))==NULL) {	      
	      /* Still no luck */
	      Log("** Warning ** : Still no luck, now we'll try 800x600 in a window.");
	      m_nDispWidth = 800; m_nDispHeight = 600;	      
	      m_bWindowed = true;
  	    if( (screen = SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,SDL_OPENGL) )==NULL) {	      
          throw Exception("SDL_SetVideoMode : " + std::string(SDL_GetError()));
  	    }	      
      }
    }
		
	  /* Retrieve actual configuration */
	  pVidInfo=SDL_GetVideoInfo();
	  if(pVidInfo==NULL)
      throw Exception("(2) SDL_GetVideoInfo : " + std::string(SDL_GetError()));
  									
	  m_nDispBPP=pVidInfo->vfmt->BitsPerPixel;


	  /* Enable unicode translation and key repeats */
	  SDL_EnableUNICODE(1);    		  
	  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	   
	  Log("Using OPEN SDL_gfx implementation");
	  
    //setBackgroundColor(0,0,40,255);
 
    /* Init the Text drawing library */
    _InitTextRendering(ptheme);

}

  void DrawLibSDLgfx::unInit(){
  }
  /*===========================================================================
  Primitive: circle
  ===========================================================================*/
  void DrawLibSDLgfx::drawCircle(const Vector2f &Center,float fRadius,float fBorder,Color Back,Color Front) {
     circleRGBA(screen,(Sint16)Center.x,(Sint16)Center.y,(Sint16)fRadius ,GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
  }
  
  /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLibSDLgfx::drawBox(const Vector2f &A,const Vector2f &B,float fBorder,Color Back,Color Front) {
   if ( GET_ALPHA(Back) > 0){
      boxRGBA(screen, A.x  , A.y , B.x  , B.y , GET_RED(Back),GET_GREEN(Back),GET_BLUE(Back),GET_ALPHA(Back));
   }

   if(fBorder>0.0f && GET_ALPHA(Front)>0) {
     boxRGBA(screen, A.x, A.y, A.x +fBorder , B.y, GET_RED(Front),GET_GREEN(Front),GET_BLUE(Front
),GET_ALPHA(Front));
  }
  }

  /*===========================================================================
  Primitive: box
  ===========================================================================*/
  void DrawLibSDLgfx::drawImage(const Vector2f &A,const Vector2f &B,Texture *pTexture,Color Tint) {
  }
        
  
  /*===========================================================================
  Check for OpenGL extension
  ===========================================================================*/
  bool DrawLibSDLgfx::isExtensionSupported(std::string Ext) {
    return false;
  }
  
    /*===========================================================================
  Grab screen contents
  ===========================================================================*/
  Img *DrawLibSDLgfx::grabScreen(void) {
    return NULL;
  }
  
  void DrawLibSDLgfx::startDraw(DrawMode mode){
    drawMode = mode;
    if(drawingPoints.size() != 0 || texturePoints.size() !=0 ){
       printf("error drawingPoints.size(%i) of texturePoints.size(%i) was not 0 =%i\n",drawingPoints.size(),texturePoints.size());
    }
  }
  
  void DrawLibSDLgfx::endDraw(){
    int size = drawingPoints.size();

    Sint16 x[size];
    Sint16 y[size];
    for (int i = 0 ; i < size ; i++){
      x[i] =  drawingPoints.at(i)->x;
      y[i] =  drawingPoints.at(i)->y;
    }
    switch(drawMode){
      case DRAW_MODE_POLYGON:
        filledPolygonRGBA(screen,x,y,size,GET_RED(m_color),GET_GREEN(m_color),GET_BLUE(m_color),GET_ALPHA(m_color));

        break;
      case DRAW_MODE_LINE_LOOP:
        polygonRGBA(screen,x,y,size,GET_RED(m_color),GET_GREEN(m_color),GET_BLUE(m_color),GET_ALPHA(m_color));
        break;
      case DRAW_MODE_LINE_STRIP:
        for (int f = 0 ; f < size -1 ; f++){
          lineRGBA(screen,x[f],y[f],x[f+1],y[f+1],GET_RED(m_color),GET_GREEN(m_color),GET_BLUE(m_color),GET_ALPHA(m_color));
        }
        break;
    } 
    drawingPoints.resize(0);
    drawMode = DRAW_MODE_NONE;
  }

  void DrawLibSDLgfx::setColor(Color color){
    m_color = color; 
  }

  void DrawLibSDLgfx::setTexture(Texture *texture,BlendMode blendMode){
  } 
  
  void DrawLibSDLgfx::setBlendMode(BlendMode blendMode){
  }



  /*===========================================================================
  Text dim probing
  ===========================================================================*/  
  int DrawLibSDLgfx::getTextHeight(std::string Text) {
  }
  
  int DrawLibSDLgfx::getTextWidth(std::string Text) {
  }

  /*===========================================================================
  Text drawing
  ===========================================================================*/  
  void DrawLibSDLgfx::drawText(const Vector2f &Pos,std::string Text,Color Back,Color Front,bool bEdge) {
  }
  
  /*===========================================================================
  Init of text rendering
  ===========================================================================*/  
  void DrawLibSDLgfx::_InitTextRendering(Theme *p_theme) {   
    m_pDefaultFontTex = p_theme->getDefaultFont();
          
    /* Create font texture (default) */
    //m_pDefaultFontTexture = (DefaultFontTexture *)pTextureManager->loadTexture(new DefaultFontTexture,"default-font");
  }
  
  /*===========================================================================
  Uninit of text rendering
  ===========================================================================*/  
  void DrawLibSDLgfx::_UninitTextRendering(Theme *p_theme) {    
  }

  /*===========================================================================
  Create a default font texture
  ===========================================================================*/  
  //void DefaultFontTexture::load(std::string Name,bool bSmall) {
  //  /* Create font object */
  //  
  //  /* Pass it to GL */
  //  GLuint N;
  //  glEnable(GL_TEXTURE_2D);
  //  glGenTextures(1,&N);    
  //  glBindTexture(GL_TEXTURE_2D,N);
  //  glTexImage2D(GL_TEXTURE_2D,0,4,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,(void *)pImgData);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  //  glDisable(GL_TEXTURE_2D);
  //  
  //  m_TI.nID = N;
//  }

  /*===========================================================================
  Unload a default font texture
  ===========================================================================*/    
  //void DefaultFontTexture::unload(void) {
 // }
      

      void DrawLibSDLgfx::clearGraphics(){
        setClipRect  (NULL);
        scale.x =1;
        scale.y =1;
        translate.x =0;
        translate.y =0;
        SDL_LockSurface(screen);
        if (bg_data ==NULL){
          bg_data = malloc(screen->format->BytesPerPixel * screen->w * screen->h);
          memset(bg_data,0,screen->format->BytesPerPixel * screen->w * screen->h);
        }
        memcpy(screen->pixels,bg_data,screen->format->BytesPerPixel * screen->w * screen->h);
        SDL_UnlockSurface(screen);
      }
      
      /**
       * Flush the graphics. In memory graphics will now be displayed
       **/
      void DrawLibSDLgfx::flushGraphics(){
        SDL_UpdateRect(screen, 0, 0, 0, 0);
      }
}
#endif
