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

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "VCommon.h"
#include "xmscene/Scene.h"
#include "gui/basic/GUI.h"
#include "XMSession.h"
#include "xmscene/BikeGhost.h"

#ifdef ENABLE_OPENGL
#include "include/xm_OpenGL.h"
#endif

class ParticlesSource;
class Universe;
class BlockVertex;
class Camera;

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
    
    unsigned int nNumVertices;
    GeomCoord *pVertices;
    GeomCoord *pTexCoords;
    
    unsigned int nVertexBufferID,nTexCoordBufferID;
  };

  struct Geom {
    Geom() {
      pTexture = NULL;
    }
    std::string material;
    std::vector<GeomPoly *> Polys;    
    Texture *pTexture;           // only used for edge Geoms
    TColor edgeBlendColor;    
    Vector2f Min,Max; /* AABB */
    bool isUpper;
  };

  /*===========================================================================
  Special effects overlay
  ===========================================================================*/
  class SFXOverlay {
    public:
      SFXOverlay() {
        m_drawLib = NULL;
#ifdef ENABLE_OPENGL	
        m_bUseShaders = false;
        m_VertShaderID = m_FragShaderID = m_ProgramID = 0;
        m_DynamicTextureID = m_FrameBufferID = 0;
#endif
        m_nOverlayWidth = m_nOverlayHeight = 0;
      }
    
      /* Methods */
      void init(DrawLib* i_drawLib, unsigned int nWidth, unsigned int nHeight);
      void cleanUp(void);
      void beginRendering(void);
      void endRendering(void);
      void fade(float f, unsigned int i_frameNumber);
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
      char **_LoadShaderSource(const std::string &File, unsigned int *pnNumLines);
      void _FreeShaderSource(char **ppc, unsigned int nNumLines);
      bool _SetShaderSource(GLhandleARB ShaderID, const std::string &File);
#endif
      
      int m_nOverlayWidth,m_nOverlayHeight;
      DrawLib* m_drawLib;
  };

  /*===========================================================================
    Game rendering class
    ===========================================================================*/
class GameRenderer {

public:
  GameRenderer();
  ~GameRenderer();

  void init(DrawLib* i_drawLib); /* only called at start-up, and not per-level */
  void render(Scene* i_scene);

  void prepareForNewLevel(Universe* i_universe);
  void unprepareForNewLevel(void);

  void loadDebugInfo(std::string File);

  float SizeMultOfEntitiesToTake() const;
  float SizeMultOfEntitiesWhichMakeWin() const;
  void setSizeMultOfEntitiesToTake(float i_sizeMult);
  void setSizeMultOfEntitiesWhichMakeWin(float i_sizeMult);    

  int nbParticlesRendered() const;
  void setBestTime(const std::string& s) {m_bestTime = s;}
  void setWorldRecordTime(const std::string &s) {m_worldRecordTime = s;}
  void setShowMinimap(bool i_value);
  void setRenderGhostTrail(bool i_value);
  bool renderGhostTrail();
  void setShowTimePanel(bool i_value);
  void setScreenShade(bool i_doShade_global, bool i_doShadeAnim_global, float i_shadeTime_global); 
  void hideReplayHelp();
  bool showEngineCounter() const;
  void setShowEngineCounter(bool i_value);
  bool showMinimap() const;
  void showReplayHelp(float p_speed, bool bAllowRewind);

  void setShowGhostsText(bool i_value);
  bool showGhostsText() const;
  void renderGameMessages(Scene* i_scene);

  static unsigned int currentRegistrationStage();
  void setGraphicsLevel();

private:
  void renderMiniMap(Scene* i_scene, int x,int y,int nWidth,int nHeight);
  void renderEngineCounter(int x,int y,int nWidth,int nHeight, float pSpeed, float pLinVel = -1);
  
  Vector2f calculateChangeDirPosition(Biker* i_biker, const Vector2f i_p);
      
  std::string getBestTime(void) {return m_bestTime;}

  DrawLib* m_drawLib;
  std::vector<GraphDebugInfo *> m_DebugInfo;
      
  std::vector<Geom*> m_StaticGeoms;
  std::vector<Geom*> m_DynamicGeoms;
  std::vector<Geom*> m_edgeGeoms;

  std::string m_bestTime;
  std::string m_replayHelp_l;
  std::string m_replayHelp_r;
  std::string m_worldRecordTime;

  GraphicsLevel m_graphicsLevel;

  bool m_allowGhostEffect; // ask to not do the ghost effect

  float m_previousEngineSpeed;
  float m_previousEngineLinVel;

  bool m_showMinimap;
  bool m_renderGhostTrail;
  bool m_showEngineCounter;
  bool m_showTimePanel;
  bool m_showGhostsText;
  
  // for Screenshadowing in menus
  bool m_doShade_global; 
  bool m_doShadeAnim_global;
  float m_nShadeTime_global;

  /* FBO overlay */
  SFXOverlay m_Overlay;

  AABB m_screenBBox;
  AABB m_layersBBox;

  float m_sizeMultOfEntitiesToTake;
  float m_sizeMultOfEntitiesWhichMakeWin;
  int   m_nParticlesRendered;
  std::string       m_currentEdgeEffect;
  TColor            m_currentEdgeBlendColor;
  EdgeEffectSprite* m_currentEdgeSprite;
  float             m_currentEdgeMaterialScale;
  float             m_currentEdgeMaterialDepth;
  Sprite*           m_currentSkySprite;
  Sprite*           m_currentSkySprite2;
  
  float m_xScale;
  float m_yScale;
  float m_xScaleDefault;
  float m_yScaleDefault;
  float m_rotationAngleForTheFrame;
  MiscSprite* m_arrowSprite;

  /* Subroutines */
	
  void setCameraTransformations(Camera* pCamera, float xScale, float yScale);
  // set m_[x,y]Scale
  void calculateCameraScaleAndScreenAABB(Camera* pCamera, AABB& bbox);

  void renderEngineCounterNeedle(int nWidth, int nHeight, Vector2f center, float value);

  void displayArrowIndication(Biker* i_biker, AABB *i_screenBBox);
  bool getBikerDirection(Biker* i_biker, AABB* i_screenBBox, Vector2f* o_arrowPoint, float* o_arrowAngle, AABBSide* o_side);
	
  void _RenderSprites(Scene* i_scene, bool bForeground,bool bBackground);
  void _RenderSprite(Scene* i_scene, Entity *pSprite, float i_sizeMult = 1.0);
  void _RenderBike(Biker* i_biker, bool i_renderBikeFront = true,
		   const TColor&  i_filterColor = TColor(255, 255, 255, 0),
		   const TColor&  i_filterUglyColor = TColor(255, 255, 255, 0));
  void renderBodyPart(const Vector2f& i_from, const Vector2f& i_to,
		      float i_c11, float i_c12,
		      float i_c21, float i_c22,
		      float i_c31, float i_c32,
		      float i_c41, float i_c42,
		      Sprite *i_sprite,
		      const TColor& i_filterColor,
		      Biker* i_biker,
		      int i_90_rotation = 0
		      );
  void _RenderBlocks(Scene* i_scene);
  void _RenderBlock(Block* block);
  void _RenderBlockEdges(Block* block);
  void _RenderDynamicBlocks(Scene* i_scene, bool bBackground=false);
  void _RenderBackground(Scene* i_scene);
  void _RenderLayers(Scene* i_scene, bool renderFront);
  void _RenderLayer(Scene* i_scene, int layer);
  void _RenderSky(Scene* i_scene, float i_zoom, float i_offset, const TColor& i_color,
		  float i_driftZoom, const TColor &i_driftColor, bool i_drifted);
  void _RenderGameMessages(Scene* i_scene, bool renderOverShadow); 
  void _RenderGameStatus(Scene* i_scene);
  void _RenderParticles(Scene* i_scene, bool bFront=true);
  void _RenderParticleDraw(Vector2f P,Texture *pTexture,float fSize,float fAngle, TColor c);
  void _RenderParticle(Scene* i_scene, ParticlesSource *i_source, unsigned int sprite=0);
  void _RenderInGameText(Vector2f P,const std::string &Text,Color c = 0xffffffff, float i_xcentering = 0.0, float i_ycentering = 0.0);
  void _RenderZone(Zone *i_zone, bool i_renderOnlyDeathZone);

  void _RenderGhost(Scene* i_scene, Biker* i_ghost, int i, float i_textOffset);
  void _RenderGhostTrail(Scene* i_scene, AABB* i_screenBBox, float i_scale);
  void _RenderDebugInfo(void);
  
  void _RenderScreenShadow(Scene* i_scene);
  void _RenderAlphaBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3,
				  const TColor& i_filterColor = TColor(255, 255, 255, 0));
  void _RenderAdditiveBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
  void _RenderAlphaBlendedSectionSP(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
  void _RenderRectangle(const Vector2f& i_p1, const Vector2f& i_p2, const Color& i_color, bool i_filled = false);
  void _RenderCircle(unsigned int nSteps, const Color i_color,const Vector2f &C,float fRadius, bool i_filled = false);
  void _deleteGeoms(std::vector<Geom *>& geom, bool useFree=false);

  void renderTimePanel(Scene* i_scene);
  void renderReplayHelpMessage(Scene* i_scene);

  Texture* loadTexture(std::string textureName);
  Texture* loadTextureEdge(std::string textureName);
  int  edgeGeomExists(Block* pBlock, std::string texture, bool isUpper);
  void initCameras(Universe* i_universe);
  int  loadBlock(Block* pBlock, Universe* i_universe, unsigned int currentScene, int sameSceneAs, int blockIndex);
  int  loadBlockGeom(Block* pBlock, std::vector<Geom *>* pGeoms, Vector2f Center, Scene* pScene);
  int  loadBlockEdge(Block* pBlock, Vector2f Center, Scene* pScene);
  void calculateEdgePosition(Block* pBlock,
			     BlockVertex* vertexA1,
			     BlockVertex* vertexB1,
			     BlockVertex* vertexC1,
			     Vector2f     center,
			     Vector2f& A1, Vector2f& B1,
			     Vector2f& B2, Vector2f& A2,
			     Vector2f& C1, Vector2f& C2,
			     Vector2f oldC2, Vector2f oldB2, bool useOld,
			     bool AisLast, bool& swapDone);
  void calculateEdgeTexture(Block* pBlock,
			    Vector2f A1,   Vector2f B1,
			    Vector2f B2,   Vector2f A2,
			    Vector2f& ua1, Vector2f& ub1,
			    Vector2f& ub2, Vector2f& ua2);

  void beginTexturesRegistration();
  void endTexturesRegistration();
  static unsigned int m_curRegistrationStage;
};

#endif
