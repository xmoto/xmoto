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

#include "StateShowcase.h"
#include "../Game.h"
#include "../drawlib/DrawLib.h"
#include "../GameText.h"
#include "../VXml.h"
#include "../helpers/Log.h"
#include "StatePreplayingGame.h"
#include "StatePreplayingCredits.h"

// HELPER
std::vector<int> readBikeParams(std::string p_bikeXml) {
  XMLDocument v_ThemeXml;
  TiXmlDocument *v_ThemeXmlData;
  TiXmlElement *v_ThemeXmlDataElement;
  std::string m_name;
  std::vector<int> v_return;

  /* open the file */
  v_ThemeXml.readFromFile(FDT_DATA, p_bikeXml);   
  v_ThemeXmlData = v_ThemeXml.getLowLevelAccess();
  
  if(v_ThemeXmlData == NULL) {
    throw Exception("error : unable to analyze xml file");
  }
  
  v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("Control"); // "xmoto_physics");
  if(v_ThemeXmlDataElement != NULL) {
    const char* pc = v_ThemeXmlDataElement->Attribute("value");
    if(pc == std::string("")) {
      v_return.push_back(0);
    }
    else {
      v_return.push_back(atoi(pc));
    }
  }
  
  
  v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("Speed"); // "xmoto_physics");
  if(v_ThemeXmlDataElement != NULL) {
    const char* pc = v_ThemeXmlDataElement->Attribute("value");
    if(pc == std::string("")) {
      v_return.push_back(0);
    }
    else {
      v_return.push_back(atoi(pc));
    }
  }
  
  v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("Overstate"); // "xmoto_physics");
  if(v_ThemeXmlDataElement != NULL) {
    const char* pc = v_ThemeXmlDataElement->Attribute("value");
    if(pc == std::string("")) {
      v_return.push_back(0);
    }
    else {
      v_return.push_back(atoi(pc));
    }
  }
  
  v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement("Stuntability"); // "xmoto_physics");
  if(v_ThemeXmlDataElement != NULL) {
    const char* pc = v_ThemeXmlDataElement->Attribute("value");
    if(pc == std::string("")) {
      v_return.push_back(0);
    }
    else {
      v_return.push_back(atoi(pc));
    }
  }
  
  return v_return;
}




StateShowcase::StateShowcase(bool drawStateBehind,
		     bool updateStatesBehind, bool i_gameShowcase, bool i_allowSceneOver):
  StateMenu(drawStateBehind,
	    updateStatesBehind, true, false)
{
  m_name  = "StateShowcase";
  m_gameShowcase = i_gameShowcase;
  m_allowSceneOver = i_allowSceneOver;
  createGUI(); // create the gui each time because it's small and keys can change
}

StateShowcase::~StateShowcase()
{
  delete m_GUI;
}

void StateShowcase::enterAfterPop() {
  StateMenu::enterAfterPop();
}

void StateShowcase::leave()
{
  StateMenu::leave();
}

void StateShowcase::checkEvents() {

  /* Close */
  UIButton *pCloseButton = (UIButton *) m_GUI->getChild("FRAME:CLOSE_BUTTON");
  if(pCloseButton->isClicked()) {
    pCloseButton->setClicked(false);

    m_requestForEnd = true;
  }
//  updateWindow();
}

void StateShowcase::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  StateMenu::xmKey(i_type, i_xmkey);

  if(i_type == INPUT_DOWN && ( i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
			       i_xmkey == XMKey(SDLK_F1,     KMOD_NONE) )) {
    m_requestForEnd = true;
  }
}

void StateShowcase::createGUI() {
  UIFrame  *v_frame;
  UIStatic *v_someText;
  GameApp* pGame = GameApp::instance();
  DrawLib* drawLib = pGame->getDrawLib();

  m_GUI = new UIRoot();
  m_GUI->setFont(drawLib->getFontSmall()); 
  m_GUI->setPosition(0, 0,
		     drawLib->getDispWidth(),
		     drawLib->getDispHeight());
  
  int v_offsetX = drawLib->getDispWidth()  / 10;
  int v_offsetY = drawLib->getDispHeight() / 10;

  v_frame = new UIFrame(m_GUI, v_offsetX, m_gameShowcase ? v_offsetY / 2 : v_offsetY, "",
			drawLib->getDispWidth()  - 2*v_offsetX,
			m_gameShowcase ? drawLib->getDispHeight() - v_offsetY : drawLib->getDispHeight() - 2*v_offsetY
			);
  v_frame->setID("FRAME");
  
  v_someText = new UIStatic(v_frame, 0, 10, "SHOWCASE", v_frame->getPosition().nWidth, 36);
  v_someText->setFont(drawLib->getFontMedium());
  v_someText = new UIStatic(v_frame, 20, 60, "Bike: ",
			    v_frame->getPosition().nWidth-20,
			    m_gameShowcase ? v_frame->getPosition().nHeight-46 :
			                 v_frame->getPosition().nHeight-56);
  v_someText->setFont(drawLib->getFontMedium());
  v_someText->setVAlign(UI_ALIGN_TOP);
  v_someText->setHAlign(UI_ALIGN_LEFT);
  
  UIButton *pCloseButton = new UIButton(v_frame, v_frame->getPosition().nWidth-120, v_frame->getPosition().nHeight-62,
					GAMETEXT_CLOSE, 115, 57);
  pCloseButton->setFont(drawLib->getFontSmall());
  pCloseButton->setType(UI_BUTTON_TYPE_SMALL);
  pCloseButton->setID("CLOSE_BUTTON");
 /* 
  UIWindow *v_window = new UIWindow(v_frame, 5, 80,"DIDDL",400,400);
  v_window->setID("PIC");
  v_window->enableWindow(true);
  v_window->setFont(GameApp::instance()->getDrawLib()->getFontSmall());
*/
    /* get the bike Image from bike theme and load it */
    
      std::string v_bikeXml = xmDatabase::instance("main")->bikes_getFileName(XMSession::instance()->bike());
  std::vector<int> v_bikeParams = readBikeParams(v_bikeXml);

  Sprite* pSprite;
  Texture* v_bikeSprite=NULL;
  v_bikeSprite = NULL;
  pSprite = ThemeManager::instance()->getTheme(XMSession::instance()->theme())->getSprite(SPRITE_TYPE_ANIMATION, "Biker");
  if(pSprite != NULL) {
    pSprite->loadTextures();
    v_bikeSprite = pSprite->getTexture(false, true, FM_LINEAR);
//    ThemeManager::instance()->getTheme(XMSession::instance()->themeBike())->loadTexture(pSprite->getCurrentTextureFileName());
  }
  else LogInfo("nichda");
//  v_frame->putImage(1,1,250,250,v_bikeSprite);

//   if(XMSession::instance()->menuGraphics() != GFX_LOW && XMSession::instance()->ugly() == false) {
    int w = drawLib->getDispWidth();
    int h = drawLib->getDispHeight();

    if(v_bikeSprite != NULL)
      drawLib->drawImage(Vector2f(0,0), Vector2f(w,h), v_bikeSprite, 0xFFFFFFFF, true);
    else LogInfo("gehd nedd");


  updateWindow(v_frame);
}

void StateShowcase::updateWindow(UIWindow* i_window) {
  std::string v_bikeXml = xmDatabase::instance("main")->bikes_getFileName(XMSession::instance()->bike());
  std::vector<int> v_bikeParams = readBikeParams(v_bikeXml);

  
  DrawLib* drawlib = GameApp::instance()->getDrawLib();
/*
//  if(XMSession::instance()->menuGraphics() != GFX_LOW && XMSession::instance()->ugly() == false) {
    int w = drawlib->getDispWidth();
    int h = drawlib->getDispHeight();

    if(v_bikeSprite != NULL)
      drawlib->drawImage(Vector2f(w/2,h/2), Vector2f(drawlib->getDispWidth(),drawlib->getDispHeight()), v_bikeSprite, 0xFFFFFFFF, true);
    else LogInfo("gehd nedd");

 // }
 */
 
 UIWindow *v_window = new UIWindow(i_window, 5, 80,"DIDDL",400,400);
  v_window->setID("PIC");
  v_window->enableWindow(true);
  v_window->setFont(GameApp::instance()->getDrawLib()->getFontSmall());
  drawlib->startDraw(DRAW_MODE_POLYGON);
//  v_window->putImage(1,1,250,250,v_bikeSprite);
  drawlib->endDraw();

  LogInfo("da hammer: %i %i %i %i",v_bikeParams[0],v_bikeParams[1],v_bikeParams[2],v_bikeParams[3]);
}

