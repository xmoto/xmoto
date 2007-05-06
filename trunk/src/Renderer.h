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
#include "xmscene/Scene.h"
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
  struct GeomCoord {
    float x,y;
  };

  struct GeomPoly {
    GeomPoly() {
      nNumVertices = 0;
      pVertices = pTexCoords = NULL;
      nVertexBufferID = nTexCoordBufferID = 0;
    }
    
    int nNumVertices;
    GeomCoord *pVertices;
    GeomCoord *pTexCoords;
    
    unsigned int nVertexBufferID,nTexCoordBufferID;
  };

  struct Geom {
    Geom() {
      pTexture = NULL;
    }
        
    std::vector<GeomPoly *> Polys;    
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
      m_displayGhostInformation = false;
      m_theme = NULL;
      m_previousEngineSpeed = -1.0;
	  m_previousEngineLinVel = -1.0;
      m_sizeMultOfEntitiesToTake = 1.0;
      m_sizeMultOfEntitiesWhichMakeWin = 1.0;
      m_playerToFollow = NULL;
      m_showMinimap = true;
      m_showEngineCounter = true;
      m_bTestThemeMode = false;
      m_bUglyOverMode  = false;

      m_mirrored = false;
      m_rotationAngle = 0.0;
    }
    ~GameRenderer() {_Free();}
    
    /* Methods */
    void init(void); /* only called at start-up, and not per-level */
    void shutdown(void);

    void setTheme(Theme *p_theme);
    void render(bool bIsPaused = false);
    void renderMiniMap(int x,int y,int nWidth,int nHeight);
    void renderEngineCounter(int x,int y,int nWidth,int nHeight, float pSpeed, float pLinVel = -1);
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
    void setUglyOverMode(bool bUglyOverMode) {m_bUglyOverMode = bUglyOverMode;}
    bool isDebug(void) {return m_bDebug;}
    UIRoot *getGUI(void) {return &m_GUI;}
    void setBestTime(std::string s) {m_pBestTime->setCaption(s);}
    void showReplayHelp(float p_speed, bool bAllowRewind);
    void hideReplayHelp();
    void setWorldRecordTime(const std::string &s) {m_pWorldRecordTime->setCaption(s);}

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
    void setGhostDisplayInformation(bool i_display);

    /* if p_save == "", nothing is displayed for p_save */
    void showMsgNewPersonalHighscore(String p_save = "");
    void showMsgNewBestHighscore(String p_save = "");
    void hideMsgNewHighscore();

    float SizeMultOfEntitiesToTake() const;
    float SizeMultOfEntitiesWhichMakeWin() const;
    void setSizeMultOfEntitiesToTake(float i_sizeMult);
    void setSizeMultOfEntitiesWhichMakeWin(float i_sizeMult);

    void setPlayerToFollow(Biker* i_playerToFollow);

    bool showMinimap() const;
    bool showEngineCounter() const;
    void setShowMinimap(bool i_value);
    void setShowEngineCounter(bool i_value);

    void switchFollow();

    bool isMirrored();
    void setMirrored(bool i_value);

    float rotationAngle();
    void setRotationAngle(float i_value);
    void setDesiredRotationAngle(float i_value);
    void adaptRotationAngleToGravity();

  private:
    /* Data */
    float m_fScale;
    float m_cameraOffsetX;
    float m_cameraOffsetY;

    bool m_mirrored;
    float m_rotationAngle;
    float m_desiredRotationAngle;
    Biker* m_playerToFollow;

    std::vector<GraphDebugInfo *> m_DebugInfo;
      
    std::vector<Geom *> m_StaticGeoms;
    std::vector<Geom *> m_DynamicGeoms;
      
    MotoGame *m_pMotoGame;        /* Game object, so we know what to draw. */
    App *m_pParent;               /* Our owner, so we know where to draw. */
      
    bool m_bDebug;
    bool m_bUglyMode;
    bool m_bTestThemeMode;
    bool m_bUglyOverMode;
    bool m_bCreditsMode;

    UIRoot m_GUI;                 /* GUI root */
      
    Theme *m_theme;

    Vector2f m_Scroll;
    float m_fZoom;
    float m_fCurrentHorizontalScrollShift;
    float m_fCurrentVerticalScrollShift;
    DriveDir m_previous_driver_dir; /* to move camera faster if the dir changed */
    bool  m_recenter_camera_fast;

    UIWindow *m_pInGameStats;
    UIStatic *m_pPlayTime;   
    UIStatic *m_pBestTime;
    UIStatic *m_pReplayHelp;
    UIStatic *m_pWorldRecordTime;

    UIStatic *m_pSpeed;      
    UIWindow *m_pInGameNewHighscore;
    UIStatic *m_pNewHighscoreBest_str;
    UIStatic *m_pNewHighscorePersonal_str;
    UIStatic *m_pNewHighscoreSave_str;

    float m_fSpeedMultiply;
      
    float m_fNextGhostInfoUpdate;
    int m_nGhostInfoTrans;
    bool m_displayGhostInformation;

    float m_previousEngineSpeed;
	float m_previousEngineLinVel;
	
    bool m_showMinimap;
    bool m_showEngineCounter;

    GraphQuality m_Quality;
    bool m_bGhostMotionBlur;
      
    /* FBO overlay */
    SFXOverlay m_Overlay;

    AABB m_screenBBox;

    float m_sizeMultOfEntitiesToTake;
    float m_sizeMultOfEntitiesWhichMakeWin;

    /* Subroutines */
	
	void renderEngineCounterNeedle(int nWidth, int nHeight, Vector2f center, float value);
	
	
    void _RenderSprites(bool bForeground,bool bBackground);
    void _RenderSprite(Entity *pSprite, float i_sizeMult = 1.0);
    void _RenderBike(BikeState *pBike, BikeParameters *pBikeParms, BikerTheme *p_theme,
		     bool i_renderBikeFront = true,
		     const TColor&  i_filterColor = TColor(255, 255, 255, 0),
		     const TColor&  i_filterUglyColor = TColor(255, 255, 255, 0));
    void renderBodyPart(const Vector2f& i_from, const Vector2f& i_to,
			float i_c11, float i_c12,
			float i_c21, float i_c22,
			float i_c31, float i_c32,
			float i_c41, float i_c42,
			Sprite *i_sprite,
			const TColor& i_filterColor,
			DriveDir i_direction,
			int i_90_rotation = 0
			);
    void _RenderBlocks(void);
    void _RenderBlock(Block* block);
    void _RenderBlockEdges(Block* block);
    void _RenderDynamicBlocks(bool bBackground=false);
    void _RenderBackground(void);
    void _RenderLayers(bool renderFront);
    void _RenderLayer(int layer);
    void _RenderSky(float i_zoom, float i_offset, const TColor& i_color,
		    float i_driftZoom, const TColor &i_driftColor, bool i_drifted);
    void _RenderGameMessages(void); 
    void _RenderGameStatus(void);
    void _RenderParticles(bool bFront=true);
    void _RenderParticleDraw(Vector2f P,Texture *pTexture,float fSize,float fAngle, TColor c);
    void _RenderParticle(ParticlesSource *i_source);
    void _RenderInGameText(Vector2f P,const std::string &Text,Color c = 0xffffffff);
    void _RenderZone(Zone *i_zone);
    void setScroll(bool isSmooth);

    void _RenderGhost(Biker* i_ghost, int i);

    void _DrawRotatedMarker(Vector2f Pos,dReal *pfRot);     
    void _RenderDebugInfo(void);      
    void guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
				    float &p_fDesiredVerticalScrollShift);

    float guessDesiredAngleRotation();

    void _RenderAlphaBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3,
				    const TColor& i_filterColor = TColor(255, 255, 255, 0));
    void _RenderAdditiveBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
    void _RenderAlphaBlendedSectionSP(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
    void _RenderRectangle(const Vector2f& i_p1, const Vector2f& i_p2, const Color& i_color);
    void _RenderCircle(int nSteps,Color CircleColor,const Vector2f &C,float fRadius);
    void _deleteGeoms(std::vector<Geom *>& geom);

    /* _Free */
    void _Free(void);
  };

}

#endif
