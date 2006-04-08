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
	Animation frame
  ===========================================================================*/
  struct AnimationFrame { 
    Texture *pTexture;              /* Texture handle */
    Vector2f Size,Center;           /* Where */
    float fDelay;                   /* How long it should be shown */
  };

	/*===========================================================================
	Animation
  ===========================================================================*/
  struct Animation {
    std::string Name;               /* Name of animation */
    std::vector<AnimationFrame *> m_Frames; /* Frames */
    
    /* Run-time info */
    double fFrameTime;
    int m_nCurFrame;
  };

	/*===========================================================================
	Sprite type
  ===========================================================================*/
  struct SpriteType {
    std::string Name;               /* Name of type */
    Vector2f Size,Center;           /* How to draw it */
    Texture *pTexture;              /* Texture */
  };

	/*===========================================================================
	Particle type
  ===========================================================================*/
  enum ParticleType {
    PT_NONE,
    PT_SMOKE1,
    PT_SMOKE2,
    PT_FIRE,
    PT_DEBRIS,
  };
  
	/*===========================================================================
	Particle
  ===========================================================================*/
  struct Particle {
    Particle() {
      Type = PT_NONE;
      bFront = true;
    }
    
    /* General */
    bool bFront;
    ParticleType Type;
    Vector2f Pos,Vel,Acc;       /* Position, velocity, and acceleration */
    float fAng,fAngVel,fAngAcc; /* Angular version of the above */
    float fSpawnTime;
    float fKillTime;
    
    /* PT_SMOKE1 / PT_SMOKE2 */
    float fSmokeSize;
    Color SmokeColor;
    
    /* PT_FIRE */
    float fFireSize;
    Color FireColor;
    float fFireSeed;
    
    /* PT_DEBRIS */
    Color DebrisTint;
    float fDebrisSize;
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
	Game rendering class
  ===========================================================================*/
  class GameRenderer {
    public:
      GameRenderer() {m_nNumSpriteTypes=0; m_bDebug=false; m_Quality=GQ_HIGH; m_fSpeedMultiply=1.0f;}
      ~GameRenderer() {_Free();}
    
      /* Methods */
      void init(void); /* only called at start-up, and not per-level */
      void render(void);
      void renderMiniMap(int x,int y,int nWidth,int nHeight);
      void prepareForNewLevel(void);
      void unprepareForNewLevel(void);
      void loadDebugInfo(std::string File);
      
      Particle *spawnParticle(ParticleType Type,Vector2f Pos,Vector2f Vel,float fLifeTime);
    
      /* Data interface */
      void setGameObject(MotoGame *pMotoGame) {m_pMotoGame=pMotoGame;}
      MotoGame *getGameObject(void) {return m_pMotoGame;}
      void setParent(App *pParent) {m_pParent=pParent;}
      App *getParent(void) {return m_pParent;}
      void setDebug(bool bDebug) {m_bDebug = bDebug;}
      void setUglyMode(bool bUglyMode) {m_bUglyMode = bUglyMode;}
      bool isDebug(void) {return m_bDebug;}
      UIRoot *getGUI(void) {return &m_GUI;}
      UIFont *getSmallFont(void) {return m_pSFont;}
      UIFont *getMediumFont(void) {return m_pMFont;}
      void setBestTime(std::string s) {m_pBestTime->setCaption(s);}
      std::string getBestTime(void) {return m_pBestTime->getCaption();}
      void setQuality(GraphQuality Quality) {m_Quality = Quality;}      
      void setSpeedMultiplier(float f) {m_fSpeedMultiply = f;}
    
    private:
      /* Data */
      std::vector<GraphDebugInfo *> m_DebugInfo;
      
      std::vector<StaticGeom *> m_Geoms;
      
      MotoGame *m_pMotoGame;        /* Game object, so we know what to draw. */
      App *m_pParent;               /* Our owner, so we know where to draw. */
      
      bool m_bDebug;
      bool m_bUglyMode;
      
      int m_nNumSpriteTypes;
      SpriteType m_SpriteTypes[100];
      
      std::vector<Animation *> m_Anims;
      UIRoot m_GUI;                 /* GUI root */
      
      Vector2f m_Scroll;
      float m_fZoom;
      float m_fDesiredHorizontalScrollShift;
      float m_fCurrentHorizontalScrollShift;
      
      UIWindow *m_pInGameStats;
      UIStatic *m_pPlayTime;   
      UIStatic *m_pBestTime;
      
      float m_fSpeedMultiply;
      
      UIFont *m_pMFont,*m_pSFont;
      
      /* Misc data */
      Texture *m_pSkyTexture1;
      Texture *m_pSkyTexture2;
      Texture *m_pSkyTexture2Drift;
      Texture *m_pEdgeGrass1;
      Animation *m_pStrawberryAnim;
      Animation *m_pFlowerAnim;
      Animation *m_pWreckerAnim;
      Texture *m_pBikeBody1;
      Texture *m_pBikeRear1;
      Texture *m_pBikeFront1;
      Texture *m_pBikeWheel1;
      Texture *m_pRiderTorso1;
      Texture *m_pRiderUpperLeg1;
      Texture *m_pRiderLowerLeg1;
      Texture *m_pRiderUpperArm1;
      Texture *m_pRiderLowerArm1;
      
      Texture *m_pArrowTexture;
      
      Texture *m_pSmoke1;
      Texture *m_pSmoke2;
      Texture *m_pFire1;
      Texture *m_pDirt1;
      
      GraphQuality m_Quality;
            
      /* Particle fun */
      float m_fNextParticleUpdate;
      std::vector<Particle *> m_Particles;
      
      /* Subroutines */
      void _RenderSprites(bool bForeground,bool bBackground);
      void _RenderSprite(Entity *pSprite);
      void _RenderBike(void);
      void _RenderBlocks(void);
      void _RenderBackground(void);
      void _RenderSky(void);
      void _RenderEntities(void);    
      void _RenderGameMessages(void); 
      void _RenderParticles(bool bFront=true);
      void _RenderParticle(Vector2f P,Texture *pTexture,float fSize,float fAngle,Color c);
      
      /* Helpers... */
      void _Vertex(Vector2f P);     /* Spit out a correctly transformed 
                                       glVertex2f() */
      SpriteType *_GetSpriteTypeByName(std::string Name);
      void _DbgText(Vector2f P,std::string Text,Color c);
      void _DrawAnimation(Vector2f P,Animation *pAnim);
      Animation *_GetAnimationByName(std::string Name);
      void _UpdateAnimations(void);
      void _UpdateParticles(float fTimeStep);
      void _DrawRotatedMarker(Vector2f Pos,dReal *pfRot);     
      void _RenderDebugInfo(void);      
      
      /* ... optimizing */
      std::vector<StaticGeom *> _FindGeomsByTexture(Texture *pTex);
      
      /* _Free */
      void _Free(void);
  };

};

#endif
