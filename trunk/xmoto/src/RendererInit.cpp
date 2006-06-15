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
 *  In-game rendering (init part)
 */
#include "VXml.h"
#include "VFileIO.h"
#include "MotoGame.h"
#include "Renderer.h"
#include "GameText.h"

namespace vapp {

  /*===========================================================================
  Init at game start-up
  ===========================================================================*/
  void GameRenderer::init(void) {
    /* Load sprite definitions. Sprites are very renderer specific in the
       meaning that they serve only a visual purpose - hence they are handled
       here in the renderer.  */
    XMLDocument Sprites;
        
    Sprites.readFromFile("sprites.dat");
    TiXmlDocument *pSprites = Sprites.getLowLevelAccess();
    if(pSprites == NULL) {
      /* No sprite definitions! */
      Log("** Warning ** : failed to load sprite definitions from 'sprites.dat'");
    }
    else {
      TiXmlElement *pSpriteDataElem = pSprites->FirstChildElement("spritedata");
      if(pSpriteDataElem == NULL) {
        /* No valid spritedata-element */
        Log("** Warning ** : 'sprites.dat' does not contain valid sprite data");
      }
      else {
        /* For each sprite */
        for(TiXmlElement *pSpriteElem=pSpriteDataElem->FirstChildElement("sprite");
            pSpriteElem!=NULL;pSpriteElem=pSpriteElem->NextSiblingElement("sprite")) {
          /* Load definition */
          const char *pc;
          std::string Name;
          pc = pSpriteElem->Attribute("name");
          if(pc != NULL) Name = pc;
          else Name = "";
          
          Vector2f Size(1,1);
          Vector2f Center(0.5,0.5);
          
          TiXmlElement *pSizeElem = pSpriteElem->FirstChildElement("size");
          TiXmlElement *pCenterElem = pSpriteElem->FirstChildElement("center");
          
          pc=pSizeElem->Attribute("width");
          if(pc != NULL) Size.x = atof(pc);

          pc=pSizeElem->Attribute("height");
          if(pc != NULL) Size.y = atof(pc);

          pc=pCenterElem->Attribute("x");
          if(pc != NULL) Center.x = atof(pc);
          else Center.x = Size.x/2.0f;

          pc=pCenterElem->Attribute("y");
          if(pc != NULL) Center.y = atof(pc);
          else Center.y = Size.y/2.0f;

          /* Load sprite */
          std::string TryPath = std::string("./Textures/Sprites/") + Name + std::string(".png");
          Texture *pTexture = getParent()->TexMan.loadTexture(TryPath,false,true,true);
          if(pTexture == NULL) {
            /* Could not load it */
            Log("** Warning ** : Sprite '%s' failed to load",TryPath.c_str());
          }
          else {
            m_SpriteTypes[m_nNumSpriteTypes].Name = Name;
            m_SpriteTypes[m_nNumSpriteTypes].Center = Center;
            m_SpriteTypes[m_nNumSpriteTypes].Size = Size;
            m_SpriteTypes[m_nNumSpriteTypes].pTexture = pTexture;
            m_nNumSpriteTypes++;
          }
        }
      }
    }
    Log(" %d sprite%s loaded",m_nNumSpriteTypes,m_nNumSpriteTypes==1?"":"s");           
    
    /* Load animations in the same way */
    XMLDocument Anims;
    int nTotalFrames = 0;
    Anims.readFromFile("anims.dat");
    TiXmlDocument *pAnims = Anims.getLowLevelAccess();
    if(pAnims == NULL) {
      /* No animation definitions! */
      Log("** Warning ** : failed to load animations from 'anims.dat'");
    }
    else {
      TiXmlElement *pAnimationDataElem = pAnims->FirstChildElement("animationdata");
      if(pAnimationDataElem == NULL) {
        /* No valid animationdata-element */
        Log("** Warning ** : 'anims.dat' does not contain valid animation data");
      }
      else {
        /* For each animation */
        for(TiXmlElement *pAnimationElem=pAnimationDataElem->FirstChildElement("animation");
            pAnimationElem!=NULL;pAnimationElem=pAnimationElem->NextSiblingElement("animation")) {
          /* Load definition */
          const char *pc;
          std::string Name;
          pc = pAnimationElem->Attribute("name");
          if(pc != NULL) Name = pc;
          else Name = "";
          
          /* Current settings (can be changed prior to each frame) */
          Vector2f Size(1,1);
          Vector2f Center(0.5,0.5);
          float fDelay=0.1f;

          /* Allocate and register animation */
          Animation *pAnim = new Animation;
          m_Anims.push_back( pAnim );
          pAnim->Name = Name;
          pAnim->fFrameTime = 0.0f;
          pAnim->m_nCurFrame = 0;
          
          /* Go trough definition... */
          for(TiXmlElement *pi=pAnimationElem->FirstChildElement();pi!=NULL;pi=pi->NextSiblingElement()) {
            std::string Type = pi->Value();
                        
            if(Type == "size") {
              pc = pi->Attribute("width");
              if(pc != NULL) Size.x = atof(pc);
              pc = pi->Attribute("height");
              if(pc != NULL) Size.y = atof(pc);
            }
            else if(Type == "center") {
              pc = pi->Attribute("x");
              if(pc != NULL) Center.x = atof(pc);
              pc = pi->Attribute("y");
              if(pc != NULL) Center.y = atof(pc);
            }
            else if(Type == "delay") {
              pc = pi->Attribute("time");
              if(pc != NULL) fDelay = atof(pc);
            }
            else if(Type == "frame") {
              pc = pi->Attribute("file");
              if(pc != NULL) {
                std::string Path = std::string("./Textures/") + std::string(pc);
              
                Texture *pTex = getParent()->TexMan.loadTexture(Path.c_str());
                if(pTex != NULL) {              
                  AnimationFrame *pFrame = new AnimationFrame;
                  pAnim->m_Frames.push_back(pFrame);
                  pFrame->pTexture = pTex;
                  pFrame->Center = Center;
                  pFrame->Size = Size;
                  pFrame->fDelay = fDelay;
                  nTotalFrames++;
                }
              }
            }
            else 
              Log("** Warning ** : unknown element '%s' in animation '%s'",Type.c_str(),Name.c_str());
          }
        }
      }
    }
    Log(" %d animation%s (%d total frame%s) loaded",m_Anims.size(),m_Anims.size()==1?"":"s",
                                                    nTotalFrames,nTotalFrames==1?"":"s");    
        
    m_pArrowTexture = getParent()->TexMan.loadTexture("Textures/Misc/Arrow.png");
            
    /* Load misc. textures */
    m_pSkyTexture1 = getParent()->TexMan.loadTexture("Textures/Effects/Sky1.jpg");
    m_pSkyTexture2 = getParent()->TexMan.loadTexture("Textures/Effects/Sky2.jpg");
    m_pSkyTexture2Drift = getParent()->TexMan.loadTexture("Textures/Effects/Sky2Drift.jpg");        
    m_pEdgeGrass1 = getParent()->TexMan.loadTexture("Textures/Effects/EdgeGrass1.png");
    
    m_pSmoke1 = getParent()->TexMan.loadTexture("Textures/Effects/Smoke1.png");
    m_pSmoke2 = getParent()->TexMan.loadTexture("Textures/Effects/Smoke2.png");
    m_pFire1 = getParent()->TexMan.loadTexture("Textures/Effects/Fire1.png");
    m_pDirt1 = getParent()->TexMan.loadTexture("Textures/Effects/Debris1.png");
    
    /* Load bike textures */
    theme_normal.BikeBody = getParent()->TexMan.loadTexture("Textures/Bikes/Body1.png");
    theme_normal.BikeRear = getParent()->TexMan.loadTexture("Textures/Bikes/Rear1.png");
    theme_normal.BikeFront = getParent()->TexMan.loadTexture("Textures/Bikes/Front1.png");
    theme_normal.BikeWheel = getParent()->TexMan.loadTexture("Textures/Bikes/Wheel1.png");
    
    if(theme_normal.BikeBody == NULL || theme_normal.BikeRear == NULL || 
       theme_normal.BikeFront == NULL || theme_normal.BikeWheel == NULL)
      throw Exception("important bike texture missing");
      
    /* Load rider textures */
    theme_normal.RiderTorso = getParent()->TexMan.loadTexture("Textures/Riders/Torso1.png");
    theme_normal.RiderUpperLeg = getParent()->TexMan.loadTexture("Textures/Riders/UpperLeg1.png");
    theme_normal.RiderLowerLeg = getParent()->TexMan.loadTexture("Textures/Riders/LowerLeg1.png");
    theme_normal.RiderUpperArm = getParent()->TexMan.loadTexture("Textures/Riders/UpperArm1.png");
    theme_normal.RiderLowerArm = getParent()->TexMan.loadTexture("Textures/Riders/LowerArm1.png");

    if(theme_normal.RiderTorso == NULL || theme_normal.RiderUpperLeg == NULL || 
       theme_normal.RiderLowerArm == NULL || theme_normal.RiderLowerLeg == NULL || theme_normal.RiderUpperArm == NULL)
      throw Exception("important rider texture missing");

#if defined(ALLOW_GHOST)
    /* Load bike textures */
    theme_ghost.BikeBody = getParent()->TexMan.loadTexture("Textures/Bikes/Body_Ghost.png");
    theme_ghost.BikeRear = getParent()->TexMan.loadTexture("Textures/Bikes/Rear_Ghost.png");
    theme_ghost.BikeFront = getParent()->TexMan.loadTexture("Textures/Bikes/Front_Ghost.png");
    theme_ghost.BikeWheel = getParent()->TexMan.loadTexture("Textures/Bikes/Wheel_Ghost.png");
    
    if(theme_ghost.BikeBody == NULL || theme_ghost.BikeRear == NULL || 
       theme_ghost.BikeFront == NULL || theme_ghost.BikeWheel == NULL)
      throw Exception("important bike texture missing");

    /* Load ghost textures */
    theme_ghost.RiderTorso = getParent()->TexMan.loadTexture("Textures/Riders/Torso_Ghost.png");
    theme_ghost.RiderUpperLeg = getParent()->TexMan.loadTexture("Textures/Riders/UpperLeg_Ghost.png");
    theme_ghost.RiderLowerLeg = getParent()->TexMan.loadTexture("Textures/Riders/LowerLeg_Ghost.png");
    theme_ghost.RiderUpperArm = getParent()->TexMan.loadTexture("Textures/Riders/UpperArm_Ghost.png");
    theme_ghost.RiderLowerArm = getParent()->TexMan.loadTexture("Textures/Riders/LowerArm_Ghost.png");

    if(theme_ghost.RiderTorso == NULL || theme_ghost.RiderUpperLeg == NULL || 
       theme_ghost.RiderLowerArm == NULL || theme_ghost.RiderLowerLeg == NULL || theme_ghost.RiderUpperArm == NULL)
      throw Exception("important rider texture missing");
#endif 
    
    /* Obtain ref. to known animations */
    m_pStrawberryAnim = _GetAnimationByName("Strawberry");
    m_pFlowerAnim = _GetAnimationByName("Flower");
    m_pWreckerAnim = _GetAnimationByName("Wrecker");
    
    if(m_pStrawberryAnim == NULL || m_pFlowerAnim == NULL || m_pWreckerAnim == NULL)
      throw Exception("important animation missing");
      
    /* Init GUI */
    getGUI()->setApp(getParent());
    getGUI()->setPosition(0,0,getParent()->getDispWidth(),getParent()->getDispHeight());
    
    m_pMFont = UITextDraw::getFont("MFont");
    m_pSFont = UITextDraw::getFont("SFont");    
    
    getGUI()->setFont(m_pSFont); /* default font */

    m_pInGameStats = new UIWindow(getGUI(),0,0,"",800,100);
    m_pInGameStats->showWindow(false);
    
    m_pPlayTime = new UIStatic(m_pInGameStats,0,0,"00:00:00",200,20);
    m_pPlayTime->setFont(m_pMFont);
    m_pPlayTime->setVAlign(UI_ALIGN_TOP);
    m_pPlayTime->setHAlign(UI_ALIGN_LEFT);
    m_pBestTime   = new UIStatic(m_pInGameStats,0,23,"--:--:-- / --:--:--",800,20);
    m_pBestTime->setFont(m_pSFont);
    m_pBestTime->setVAlign(UI_ALIGN_TOP);
    m_pBestTime->setHAlign(UI_ALIGN_LEFT);
    m_pBestTime->setContextHelp("Personal best time / best time on this computer");
    m_pReplayHelp = new UIStatic(m_pInGameStats, 200, 0, "", 590, 20);
    m_pReplayHelp->setFont(m_pSFont);
    m_pReplayHelp->setVAlign(UI_ALIGN_TOP);
    m_pReplayHelp->setHAlign(UI_ALIGN_RIGHT);
#if defined(SUPPORT_WEBACCESS) 
    m_pWorldRecordTime = new UIStatic(m_pInGameStats,0,43,"",800,20);
    m_pWorldRecordTime->setFont(m_pSFont);
    m_pWorldRecordTime->setVAlign(UI_ALIGN_TOP);
    m_pWorldRecordTime->setHAlign(UI_ALIGN_LEFT);
#endif
    m_pSpeed = new UIStatic(m_pInGameStats,0,60,"",60,20);
    m_pSpeed->setFont(m_pSFont);
    m_pSpeed->setVAlign(UI_ALIGN_TOP);
    m_pSpeed->setHAlign(UI_ALIGN_RIGHT);

    /* new highscore ! */
    m_pInGameNewHighscore = new UIWindow(getGUI(),405,475,"",200,100);
    m_pInGameNewHighscore->showWindow(false);

    m_pNewHighscorePersonal_str = new UIStatic(m_pInGameNewHighscore,
					       0, 5,
					       GAMETEXT_NEWHIGHSCOREPERSONAL,
					       200, 20);
    m_pNewHighscorePersonal_str->setFont(m_pSFont);
    m_pNewHighscorePersonal_str->setHAlign(UI_ALIGN_CENTER);
    m_pNewHighscorePersonal_str->showWindow(false);

    m_pNewHighscoreBest_str = new UIStatic(m_pInGameNewHighscore,
					   0, 0,
					   GAMETEXT_NEWHIGHSCORE,
					   200, 30);
    m_pNewHighscoreBest_str->setFont(m_pMFont);
    m_pNewHighscoreBest_str->setHAlign(UI_ALIGN_CENTER);
    m_pNewHighscoreBest_str->showWindow(false);

    m_pNewHighscoreSave_str = new UIStatic(m_pInGameNewHighscore,
					   0, 25,
					   "",
					   200, 20);
    m_pNewHighscoreSave_str->setFont(m_pSFont);
    m_pNewHighscoreSave_str->setHAlign(UI_ALIGN_CENTER);
    m_pNewHighscoreSave_str->showWindow(false);

    /* Overlays? */
    m_Overlay.init(getParent(),512,512);
  }

};

