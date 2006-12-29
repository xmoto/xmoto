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

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "VCommon.h"
#include "VApp.h"
#include "MotoGame.h"
#include "GUI.h"
#include "Replay.h"

#define ZOOM_DEFAULT 0.195f
#define CAMERA_OFFSETX_DEFAULT 0.0
#define CAMERA_OFFSETY_DEFAULT 0.0

namespace vapp {
 
  /*===========================================================================
  Quality settings
  ===========================================================================*/
  enum GraphQuality {
    GQ_LOW,GQ_MEDIUM,GQ_HIGH    
  };

  /*===========================================================================
  Graphical debug info
  ===========================================================================*/
  struct GraphDebugInfo {
    std::string Type;
    std::vector<std::string> Args;    
  };

  /*===========================================================================
  Static geometry
  ===========================================================================*/
  struct StaticGeomCoord {
    float x,y;
  };

  struct StaticGeomPoly {
    StaticGeomPoly() {
      nNumVertices = 0;
      pVertices = pTexCoords = NULL;
      nVertexBufferID = nTexCoordBufferID = 0;
    }
    
    int nNumVertices;
    StaticGeomCoord *pVertices;
    StaticGeomCoord *pTexCoords;
    
    unsigned int nVertexBufferID,nTexCoordBufferID;
  };

  struct StaticGeom {
    StaticGeom() {
      pTexture = NULL;
    }
        
    std::vector<StaticGeomPoly *> Polys;    
    Texture *pTexture;
    
    Vector2f Min,Max; /* AABB */
  };

  /*===========================================================================
  Special effects overlay
  ===========================================================================*/
  class SFXOverlay {
    public:
      SFXOverlay() {
        m_pApp = NULL;
#ifdef ENABLE_OPENGL	
        m_bUseShaders = false;
        m_VertShaderID = m_FragShaderID = m_ProgramID = 0;
        m_DynamicTextureID = m_FrameBufferID = 0;
#endif
        m_nOverlayWidth = m_nOverlayHeight = 0;
      }
    
      /* Methods */
      void init(App *pApp,int nWidth,int nHeight);
      void cleanUp(void);
      void beginRendering(void);
      void endRendering(void);
      void fade(float f);
      void present(void);
    
    private:
#ifdef ENABLE_OPENGL
      /* Some OpenGL handles */
      GLuint m_DynamicTextureID;
      GLuint m_FrameBufferID;      

      /* For shaders */
      bool m_bUseShaders;
      GLhandleARB m_VertShaderID;
      GLhandleARB m_FragShaderID;
      GLhandleARB m_ProgramID;

      /* Helpers */
      char **_LoadShaderSource(const std::string &File,int *pnNumLines);
      void _FreeShaderSource(char **ppc,int nNumLines);
      bool _SetShaderSource(GLhandleARB ShaderID,const std::string &File);
#endif
      
      

      int m_nOverlayWidth,m_nOverlayHeight;
      App *m_pApp;
  };

  /*===========================================================================
  Game rendering class
  ===========================================================================*/
  class GameRenderer {
    public:
      GameRenderer() {
  m_bDebug=false;
  m_Quality=GQ_HIGH;
  m_fSpeedMultiply=1.0f;
  m_fScale = ZOOM_DEFAULT;
  m_cameraOffsetX = CAMERA_OFFSETX_DEFAULT;
  m_cameraOffsetY = CAMERA_OFFSETY_DEFAULT;
  m_bGhostMotionBlur = true;
  m_theme = NULL;
  m_previousEngineSpeed = -1.0;
  m_renderBikeFront = true;
      }
      ~GameRenderer() {_Free();}
    
      /* Methods */
      void init(void); /* only called at start-up, and not per-level */
      void shutdown(void);
      
      void setTheme(Theme *p_theme);
      void render(bool bIsPaused = false);
      void renderMiniMap(int x,int y,int nWidth,int nHeight);
      void renderEngineCounter(int x,int y,int nWidth,int nHeight, float pSpeed);
      void prepareForNewLevel(bool bCreditsMode=false);
      void unprepareForNewLevel(void);
      void loadDebugInfo(std::string File);
      
      /* Data interface */
      void setGameObject(MotoGame *pMotoGame) {m_pMotoGame=pMotoGame;}
      MotoGame *getGameObject(void) {return m_pMotoGame;}
      void setParent(App *pParent) {m_pParent=pParent;}
      App *getParent(void) {return m_pParent;}
      void setDebug(bool bDebug) {m_bDebug = bDebug;}
      void setUglyMode(bool bUglyMode) {m_bUglyMode = bUglyMode;}
      void setTestThemeMode(bool bTestThemeMode) {m_bTestThemeMode = bTestThemeMode;}
      bool isDebug(void) {return m_bDebug;}
      UIRoot *getGUI(void) {return &m_GUI;}
      UIFont *getSmallFont(void) {return m_pSFont;}
      UIFont *getMediumFont(void) {return m_pMFont;}
      void setBestTime(std::string s) {m_pBestTime->setCaption(s);}
      void showReplayHelp(float p_speed, bool bAllowRewind);
      void hideReplayHelp();
#if defined(SUPPORT_WEBACCESS)  
      void setWorldRecordTime(const std::string &s) {m_pWorldRecordTime->setCaption(s);}
#endif
      void setSpeed(const std::string &s) {m_pSpeed->setCaption(s);}
      std::string getBestTime(void) {return m_pBestTime->getCaption();}
      void setQuality(GraphQuality Quality) {m_Quality = Quality;}      
      void setSpeedMultiplier(float f) {m_fSpeedMultiply = f;}
      void zoom(float p_f);
      void setZoom(float p_f);
      void initZoom();
      float getCurrentZoom();
      void moveCamera(float px, float py);
      void setCameraPosition(float px, float py);
      float getCameraPositionX();
      float getCameraPositionY();
      void initCamera();
      void initCameraPosition();
      void setGhostMotionBlur(bool b) {m_bGhostMotionBlur = b;}
      
#if defined(ALLOW_GHOST)
      void setGhostReplay(Replay *pReplay) {m_pGhostReplay = pReplay;}
      void setGhostReplayDesc(const std::string &s) {m_ReplayDesc = s;}
#endif

      /* if p_save == "", nothing is displayed for p_save */
      void showMsgNewPersonalHighscore(String p_save = "");
      void showMsgNewBestHighscore(String p_save = "");
      void hideMsgNewHighscore();

      void setRenderBikeFront(bool state) { m_renderBikeFront = state;}

    private:
      /* Data */
      float m_fScale;
      float m_cameraOffsetX;
      float m_cameraOffsetY;

      std::vector<GraphDebugInfo *> m_DebugInfo;
      
      std::vector<StaticGeom *> m_Geoms;
      
      MotoGame *m_pMotoGame;        /* Game object, so we know what to draw. */
      App *m_pParent;               /* Our owner, so we know where to draw. */
      
      bool m_bDebug;
      bool m_bUglyMode;
      bool m_bTestThemeMode;
      bool m_bCreditsMode;

      UIRoot m_GUI;                 /* GUI root */
      
      Theme *m_theme;

      Vector2f m_Scroll;
      float m_fZoom;
      float m_fCurrentHorizontalScrollShift;
      float m_fCurrentVerticalScrollShift;
      DriveDir m_previous_driver_dir; /* to move camera faster if the dir changed */
      bool  m_recenter_camera_fast;

      bool m_renderBikeFront;

      UIWindow *m_pInGameStats;
      UIStatic *m_pPlayTime;   
      UIStatic *m_pBestTime;
      UIStatic *m_pReplayHelp;
#if defined(SUPPORT_WEBACCESS)  
      UIStatic *m_pWorldRecordTime;
#endif
      UIStatic *m_pSpeed;      
      UIWindow *m_pInGameNewHighscore;
      UIStatic *m_pNewHighscoreBest_str;
      UIStatic *m_pNewHighscorePersonal_str;
      UIStatic *m_pNewHighscoreSave_str;

      float m_fSpeedMultiply;
      
      UIFont *m_pMFont,*m_pSFont;

#if defined(ALLOW_GHOST)
      Vector2f m_GhostInfoPos,m_GhostInfoVel;
      float m_fNextGhostInfoUpdate;
      int m_nGhostInfoTrans;
      std::string m_GhostInfoString;
      Replay *m_pGhostReplay;
      std::string m_ReplayDesc;
#endif      
      
      float m_previousEngineSpeed;

      GraphQuality m_Quality;
      bool m_bGhostMotionBlur;
      
      /* FBO overlay */
      SFXOverlay m_Overlay;
            
      /* Subroutines */
      void _RenderSprites(bool bForeground,bool bBackground);
      void _RenderSprite(Entity *pSprite);
      void _RenderBike(BikeState *pBike, BikeParameters *pBikeParms, BikerTheme *p_theme);
      void _RenderBlocks(void);
      void _RenderDynamicBlocks(bool bBackground=false);
      void _RenderBackground(void);
      void _RenderSky(void);
      void _RenderGameMessages(void); 
      void _RenderGameStatus(void);
      void _RenderParticles(bool bFront=true);
      void _RenderParticleDraw(Vector2f P,Texture *pTexture,float fSize,float fAngle, TColor c);
      void _RenderParticle(ParticlesSource *i_source);
      void _RenderInGameText(Vector2f P,const std::string &Text,Color c = 0xffffffff);
      void setScroll(bool isSmooth);

      void _DbgText(Vector2f P,std::string Text,Color c);
      void _DrawRotatedMarker(Vector2f Pos,dReal *pfRot);     
      void _RenderDebugInfo(void);      
      void guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
              float &p_fDesiredVerticalScrollShift);

      void _RenderAlphaBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
      void _RenderAdditiveBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
      void _RenderAlphaBlendedSectionSP(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
      void _RenderCircle(int nSteps,Color CircleColor,const Vector2f &C,float fRadius);

      /* ... optimizing */
      std::vector<StaticGeom *> _FindGeomsByTexture(Texture *pTex);
      
      /* _Free */
      void _Free(void);
  };

}

#endif
