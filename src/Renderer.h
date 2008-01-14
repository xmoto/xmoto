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

class ParticlesSource;
class Universe;
class BlockVertex;

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
class GameRenderer : public Singleton<GameRenderer> {
  friend class Singleton<GameRenderer>;

public:
  void init(DrawLib* i_drawLib); /* only called at start-up, and not per-level */
  void shutdown(void);
  void render(MotoGame* i_scene);

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
  void setShowTimePanel(bool i_value);
  void hideReplayHelp();
  bool showEngineCounter() const;
  void setShowEngineCounter(bool i_value);
  bool showMinimap() const;
  void showReplayHelp(float p_speed, bool bAllowRewind);
  void switchFollow(MotoGame* i_scene);

private:
  void renderMiniMap(MotoGame* i_scene, int x,int y,int nWidth,int nHeight);
  void renderEngineCounter(int x,int y,int nWidth,int nHeight, float pSpeed, float pLinVel = -1);
      
  std::string getBestTime(void) {return m_bestTime;}

  GameRenderer();
  ~GameRenderer();

  DrawLib* m_drawLib;
  std::vector<GraphDebugInfo *> m_DebugInfo;
      
  std::vector<Geom*> m_StaticGeoms;
  std::vector<Geom*> m_DynamicGeoms;
  std::vector<Geom*> m_edgeGeoms;

  std::string m_bestTime;
  std::string m_replayHelp;
  std::string m_worldRecordTime;

  bool m_allowGhostEffect; // ask to not do the ghost effect

  float m_fNextGhostInfoUpdate;
  int m_nGhostInfoTrans;

  float m_previousEngineSpeed;
  float m_previousEngineLinVel;

  bool m_showMinimap;
  bool m_showEngineCounter;
  bool m_showTimePanel;

  /* FBO overlay */
  SFXOverlay m_Overlay;

  AABB m_screenBBox;

  float m_sizeMultOfEntitiesToTake;
  float m_sizeMultOfEntitiesWhichMakeWin;
  int   m_nParticlesRendered;
  std::string       m_currentEdgeEffect;
  EdgeEffectSprite* m_currentEdgeSprite;


  /* Subroutines */
	
  void renderEngineCounterNeedle(int nWidth, int nHeight, Vector2f center, float value);
	
	
  void _RenderSprites(MotoGame* i_scene, bool bForeground,bool bBackground);
  void _RenderSprite(MotoGame* i_scene, Entity *pSprite, float i_sizeMult = 1.0);
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
  void _RenderBlocks(MotoGame* i_scene);
  void _RenderBlock(Block* block);
  void _RenderBlockEdges(Block* block);
  void _RenderDynamicBlocks(MotoGame* i_scene, bool bBackground=false);
  void _RenderBackground(MotoGame* i_scene);
  void _RenderLayers(MotoGame* i_scene, bool renderFront);
  void _RenderLayer(MotoGame* i_scene, int layer);
  void _RenderSky(MotoGame* i_scene, float i_zoom, float i_offset, const TColor& i_color,
		  float i_driftZoom, const TColor &i_driftColor, bool i_drifted);
  void _RenderGameMessages(MotoGame* i_scene); 
  void _RenderGameStatus(MotoGame* i_scene);
  void _RenderParticles(MotoGame* i_scene, bool bFront=true);
  void _RenderParticleDraw(Vector2f P,Texture *pTexture,float fSize,float fAngle, TColor c);
  void _RenderParticle(MotoGame* i_scene, ParticlesSource *i_source);
  void _RenderInGameText(Vector2f P,const std::string &Text,Color c = 0xffffffff);
  void _RenderZone(Zone *i_zone);

  void _RenderGhost(MotoGame* i_scene, Biker* i_ghost, int i);

  void _DrawRotatedMarker(Vector2f Pos,dReal *pfRot);     
  void _RenderDebugInfo(void);

  void _RenderAlphaBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3,
				  const TColor& i_filterColor = TColor(255, 255, 255, 0));
  void _RenderAdditiveBlendedSection(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
  void _RenderAlphaBlendedSectionSP(Texture *pTexture,const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3);
  void _RenderRectangle(const Vector2f& i_p1, const Vector2f& i_p2, const Color& i_color);
  void _RenderCircle(unsigned int nSteps,Color CircleColor,const Vector2f &C,float fRadius);
  void _deleteGeoms(std::vector<Geom *>& geom, bool useFree=false);

  void renderTimePanel(MotoGame* i_scene);
  void renderReplayHelpMessage(MotoGame* i_scene);

  Texture* loadTexture(std::string textureName);
  Texture* loadTextureEdge(std::string textureName);
  int  edgeGeomExists(Block* pBlock, std::string texture);
  void initCameras(Universe* i_universe);
  int  loadBlock(Block* pBlock, Universe* i_universe, unsigned int currentScene, int sameSceneAs, int blockIndex);
  int  loadBlockGeom(Block* pBlock, std::vector<Geom *>* pGeoms, Texture* pTexture, Vector2f Center, MotoGame* pScene);
  int  loadBlockEdge(Block* pBlock, Vector2f Center, MotoGame* pScene);
  void calculateEdgePosition(Block* pBlock,
			     BlockVertex* vertexA1,
			     BlockVertex* vertexB1,
			     BlockVertex* vertexC1,
			     Vector2f     center,
			     Vector2f& A1, Vector2f& B1,
			     Vector2f& B2, Vector2f& A2,
			     Vector2f& C1, Vector2f& C2,
			     Vector2f oldC2, Vector2f oldB2, bool useOld);
  void calculateEdgeTexture(Block* pBlock,
			    Vector2f A1, Vector2f B1,
			    Vector2f B2, Vector2f A2,
			    float* ua1, float* ub1,
			    float* ub2, float* ua2);
};

#endif
