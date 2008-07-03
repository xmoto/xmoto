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

/* 
 *  In-game graphical user interface
 */
#include "VXml.h"
#include "GUI.h"
#include "GameText.h"
#include "drawlib/DrawLib.h"
#include "Game.h"
#include "helpers/Log.h"

  DrawLib* UIWindow::m_drawLib = NULL;

  void UIWindow::setDrawLib(DrawLib* i_drawLib) {
    m_drawLib = i_drawLib;
  }

  /*===========================================================================
  Calculate full opacity
  ===========================================================================*/
  float UIWindow::getOpacity(void) {
    float fOpacity = 100.0f;
    for(UIWindow *pWindow=this;pWindow!=NULL;pWindow=pWindow->getParent()) {
      fOpacity *= pWindow->getLocalOpacity()/100.0f;
    }
    return fOpacity;
  }
  
bool UIWindow::isUglyMode() {
  return XMSession::instance()->ugly();
}

  /*===========================================================================
  Base window init/shutdown
  ===========================================================================*/
  bool UIWindow::haveChildW(UIWindow *pWindow) {
    for(unsigned int i=0;i<m_Children.size();i++)
      if(m_Children[i] == pWindow) return true;
    return false;
  }
  
  void UIWindow::addChildW(UIWindow *pWindow) {    
    if(!haveChildW(pWindow))
      m_Children.push_back(pWindow);
  }    
  
  void UIWindow::removeChildW(UIWindow *pWindow) {
    for(unsigned int i=0;i<m_Children.size();i++) {
      if(m_Children[i] == pWindow) {
        m_Children.erase(m_Children.begin() + i);
        return;
      }
    }
  }
  
  void UIWindow::initW(UIWindow *pParent,int x,int y,std::string Caption,int nWidth,int nHeight) {
    _InitWindow();

    setPosition(x,y,nWidth,nHeight);
    setCaption(Caption);
    setOpacity(100.0f);
    m_pParent = pParent;
    getParent()->addChildW(this);
  } 
  
  void UIWindow::freeW(void) {
    if(getParent())
      getParent()->removeChildW(this);
      
    while(!m_Children.empty())
      delete m_Children[0];
  } 

  /*===========================================================================
  Base utils
  ===========================================================================*/
  void UIWindow::showWindow(bool b) {  
    m_bHide=!b; 
    if(!m_bHide && m_pPrimaryChild!=NULL) {
      getRoot()->deactivate(getRoot()); 
      m_pPrimaryChild->setActive(true);
    }
  }
  
  bool UIWindow::isActive() {
    return m_bActive;
  }

  UIRoot *UIWindow::getRoot(void) {
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      if(p->getParent() == NULL) {
        return reinterpret_cast<UIRoot *>(p);
      }
    }    
    
    return NULL;
  }
  
  void UIWindow::enableChildren(bool bState) {
    for(unsigned int i=0;i<m_Children.size();i++)
      m_Children[i]->enableWindow(bState);
  }

  void UIWindow::makeActive(void) {
    /* Find root */
    UIRoot *pRoot = NULL;
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      if(p->getParent() == NULL) {
        pRoot = reinterpret_cast<UIRoot *>(p);
        break;
      }
    }    
    
    if(pRoot != NULL) {
      pRoot->deactivate(pRoot);
      setActive(true);
    }
  }
  
  UIWindow *UIWindow::getChild(std::string Child) {
    /* The given string is a child path in the form "Child:GrandChild:undsoweiter" */
    int s = Child.find_first_of(":");
    std::string X="",XRem="";
    if(s < 0) {      
      X = Child;
      XRem = "";
    }
    else {
      X = Child.substr(0,s);
      XRem = Child.substr(s+1,Child.length()-s-1);
    }
    
    for(unsigned int i=0;i<getChildren().size();i++) {
      if(getChildren()[i]->getID() == X) {
        if(XRem != "")
	  return getChildren()[i]->getChild(XRem);
        else
	  return getChildren()[i];
      }
    }
    return NULL;
  }

  int UIWindow::getAbsPosX(void) {
    int i=0;
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      i+=p->getPosition().nX;
    }
    return i;
  }
  
  int UIWindow::getAbsPosY(void) {
    int i=0;
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      i+=p->getPosition().nY;
    }
    return i;
  }
  
  bool UIWindow::isDisabled(void) {
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      if(p->m_bDisable) {
        return true;
      }
    }    
    return false; /* nope :) */
  }

  bool UIWindow::isVisible() {
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      if(p->m_bHide) {
        return false;
      }
    }    
    return true;
  }

  bool UIWindow::isBranchHidden(void) {
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      if(p->m_bHide) {
        return true;
      }
    }    
    return false;
  }
  
  /*===========================================================================
  Special message box for querying keypresses
  ===========================================================================*/    
  bool UIQueryKeyBox::keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char) {
    //    MessageBox(NULL,"HELLO",NULL,MB_OK);
    return false;
  }
  
  /*===========================================================================
  Message boxes
  ===========================================================================*/  
  UIMsgBoxButton UIMsgBox::getClicked(void) {
    /* Go through buttons... anything clicked? */
    for(unsigned int i=0;i<m_nNumButtons;i++) {
      if(m_pButtons[i]->isClicked()) {  
        if(m_pButtons[i]->getCaption() == GAMETEXT_OK)
	  return UI_MSGBOX_OK;
        if(m_pButtons[i]->getCaption() == GAMETEXT_CANCEL)
	  return UI_MSGBOX_CANCEL;
        if(m_pButtons[i]->getCaption() == GAMETEXT_YES)
	  return UI_MSGBOX_YES;
        if(m_pButtons[i]->getCaption() == GAMETEXT_NO)
	  return UI_MSGBOX_NO;
        if(m_pButtons[i]->getCaption() == GAMETEXT_YES_FOR_ALL)
	  return UI_MSGBOX_YES_FOR_ALL;
      }
    }
    return UI_MSGBOX_NOTHING;
  }
  
  bool UIMsgBox::setClicked(std::string Text) {
    bool b = false;
    for(unsigned int i=0; i<m_nNumButtons; i++) {
      if(m_pButtons[i]->getCaption() == Text) {
        m_pButtons[i]->setClicked(true);
        b = true;
      }
    }
    return b;
  }
  
  void UIMsgBox::_ReEnableSiblings(void) {
    UIWindow *pParent = getParent();
    for(unsigned int i=0;i<pParent->getChildren().size()-1;i++) {
      pParent->getChildren()[i]->enableWindow(m_SiblingStates[i]);
    }
  }

void UIMsgBox::makeActiveButton(UIMsgBoxButton i_button) {
  for(unsigned int i=0; i<m_nNumButtons; i++) {

    if(i_button == UI_MSGBOX_OK) {
      if(m_pButtons[i]->getCaption() == GAMETEXT_OK) {
	m_pButtons[i]->makeActive();
	return;
      }
    }

   if(i_button == UI_MSGBOX_CANCEL) {
      if(m_pButtons[i]->getCaption() == GAMETEXT_CANCEL) {
	m_pButtons[i]->makeActive();
	return;
      }
    }

   if(i_button == UI_MSGBOX_YES) {
      if(m_pButtons[i]->getCaption() == GAMETEXT_YES) {
	m_pButtons[i]->makeActive();
	return;
      }
    }

   if(i_button == UI_MSGBOX_NO) {
      if(m_pButtons[i]->getCaption() == GAMETEXT_NO) {
	m_pButtons[i]->makeActive();
	return;
      }
    }

   if(i_button == UI_MSGBOX_YES_FOR_ALL) {
      if(m_pButtons[i]->getCaption() == GAMETEXT_YES_FOR_ALL) {
	m_pButtons[i]->makeActive();
	return;
      }
    }

  }
}

  void UIMsgBox::paint(void) {
    /* Should the OK button be disabled? (if any) */
    if(m_bTextInput && m_TextInput.empty()) {
      for(unsigned int i=0; i<m_nNumButtons; i++) {
        if(m_pButtons[i]->getCaption() == GAMETEXT_OK)
	  m_pButtons[i]->enableWindow(false);
      }
    }
    else {
      for(unsigned int i=0; i<m_nNumButtons; i++) {
        if(m_pButtons[i]->getCaption() == GAMETEXT_OK)
	  m_pButtons[i]->enableWindow(true);
      }
    }   
  
    /* First draw frame */
    UIFrame::paint();
    
    /* Should we do some text? */
    if(m_bTextInput && m_textInputFont!=NULL) {      
      setFont(m_textInputFont);
      putText(16,120,m_TextInput + std::string("|"));
    }
  }
  
  bool UIMsgBox::keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char) {
    switch(nKey) {
      case SDLK_ESCAPE:
        if(!setClicked(GAMETEXT_CANCEL))
          setClicked(GAMETEXT_NO);
        return true;
      case SDLK_RETURN:
        if(!m_bTextInput || !m_TextInput.empty()) {
          setClicked(GAMETEXT_OK);
          return true;
        }
        break;
      default:
        if(m_bTextInput) {
          switch(nKey) {
            case SDLK_BACKSPACE:
              if(!m_TextInput.empty()) {
                m_TextInput.erase(m_TextInput.end()-1);
              }
              return true;
            default:
              if(nChar) {
                char c[2];
                c[0] = nChar;
                c[1] = '\0';
                m_TextInput.append(c);
              }
              return true;
          }
        }
        break;
    }
    
    return false;
  }
  
  UIMsgBox *UIWindow::msgBox(std::string Text,UIMsgBoxButton Buttons,bool bTextInput,bool bQuery) {
    unsigned int nNumButtons = 0;
    
    if(Buttons & UI_MSGBOX_OK)
      nNumButtons++;
    if(Buttons & UI_MSGBOX_CANCEL)
      nNumButtons++;
    if(Buttons & UI_MSGBOX_YES)
      nNumButtons++;
    if(Buttons & UI_MSGBOX_NO)
      nNumButtons++;
    if(Buttons & UI_MSGBOX_YES_FOR_ALL)
      nNumButtons++;

    /* Determine size of contents */
    FontGlyph* v_fg = m_curFont->getGlyph(Text);

    const unsigned int nButtonSize = 57;
    int w = (v_fg->realWidth() > nNumButtons*115 ? (v_fg->realWidth()) : nNumButtons*115) + 16 + 100;
    int h = v_fg->realHeight() + nButtonSize + 24 + 100;
    
    if(bTextInput) h+=40;    
    
    /* Create the box */
    UIMsgBox *pMsgBox;
    if(bQuery) pMsgBox = (UIMsgBox *)new UIQueryKeyBox(this,getPosition().nWidth/2 - w/2,getPosition().nHeight/2 - h/2,"",w,h);
    else pMsgBox = new UIMsgBox(this,getPosition().nWidth/2 - w/2,getPosition().nHeight/2 - h/2,"",w,h);
    
    pMsgBox->makeActive();    
    if(bTextInput)
      pMsgBox->enableTextInput();
    
    /* Disable all siblings (if there's not already a msg-box) */
    //if(!m_bActiveMsgBox) {
      for(unsigned int i=0;i<getChildren().size()-1;i++) {      
        pMsgBox->getSiblingStates().push_back(!getChildren()[i]->isDisabled());
        getChildren()[i]->enableWindow(false);     
      }
    //}
    
    /* Create text static */
    int nStaticY=0;
    if(bTextInput) nStaticY=40;
    UIStatic *pText = new UIStatic(pMsgBox,8,8,Text,pMsgBox->getPosition().nWidth-16,
                                   pMsgBox->getPosition().nHeight-24-nButtonSize-nStaticY);   
    pText->setFont(getFont()); /* inherit font */      
    pText->setBackgroundShade(true); /* make text more easy to read */
    
    /* Create buttons */
    int nCX = pMsgBox->getPosition().nWidth/2 - (nNumButtons*115)/2;
    int nCY = pMsgBox->getPosition().nHeight-16-nButtonSize;
    
    UIButton *pButton;
    if(Buttons & UI_MSGBOX_OK) {
      pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_OK,115,57);
      pButton->setFont(getFont());
      pButton->setType(UI_BUTTON_TYPE_SMALL);
      pMsgBox->addButton(pButton);
      nCX+=115;
    }
    if(Buttons & UI_MSGBOX_CANCEL) {
      pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_CANCEL,115,57);
      pButton->setFont(getFont());
      pButton->setType(UI_BUTTON_TYPE_SMALL);
      pMsgBox->addButton(pButton);
      nCX+=115;
    }
    if(Buttons & UI_MSGBOX_YES) {
      pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_YES,115,57);
      pButton->setFont(getFont());
      pButton->setType(UI_BUTTON_TYPE_SMALL);
      pMsgBox->addButton(pButton);
      nCX+=115;
    }
    if(Buttons & UI_MSGBOX_NO) {
      pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_NO,115,57);
      pButton->setFont(getFont());
      pButton->setType(UI_BUTTON_TYPE_SMALL);
      pMsgBox->addButton(pButton);
      nCX+=115;
    }
    if(Buttons & UI_MSGBOX_YES_FOR_ALL) {
      pButton = new UIButton(pMsgBox,nCX,nCY,GAMETEXT_YES_FOR_ALL,115,57);
      pButton->setFont(getFont());
      pButton->setType(UI_BUTTON_TYPE_SMALL);
      pMsgBox->addButton(pButton);
      nCX+=115;
    }    

    /* Set msgbox flag */
    m_bActiveMsgBox = true;

    return pMsgBox;                                
  }

  /*===========================================================================
  Base painting
  ===========================================================================*/
  void UIWindow::putTextS(int x,int y, std::string Text,
			  int& o_width, int &o_height,
			  float i_xper, float i_yper) {
    if(m_curFont == NULL)
      return;

    /* Draw text at location */
    Color c0 = MAKE_COLOR(GET_RED(getTextStyle().c0),
			  GET_GREEN(getTextStyle().c0),
			  GET_BLUE(getTextStyle().c0),
			  (int)(GET_ALPHA(getTextStyle().c0)*getOpacity()/100));
    Color c1 = MAKE_COLOR(GET_RED(getTextStyle().c1),
			  GET_GREEN(getTextStyle().c1),
			  GET_BLUE(getTextStyle().c1),
			  (int)(GET_ALPHA(getTextStyle().c1)*getOpacity()/100));
    Color c2 = MAKE_COLOR(GET_RED(getTextStyle().c2),
			  GET_GREEN(getTextStyle().c2),
			  GET_BLUE(getTextStyle().c2),
			  (int)(GET_ALPHA(getTextStyle().c2)*getOpacity()/100));
    Color c3 = MAKE_COLOR(GET_RED(getTextStyle().c3),
			  GET_GREEN(getTextStyle().c3),
			  GET_BLUE(getTextStyle().c3),
			  (int)(GET_ALPHA(getTextStyle().c3)*getOpacity()/100));
    
    FontManager* v_fm = m_curFont;
    FontGlyph* v_fg = v_fm->getGlyph(Text);
    v_fm->printStringGrad(v_fg,
			  getAbsPosX()+x + (int)(((float)v_fg->realWidth()) * i_xper),
			  getAbsPosY()+y + (int)(((float)v_fg->realHeight()) * i_yper),
			  c0, c1, c2, c3);
    o_width  = v_fg->realWidth();
    o_height = v_fg->realHeight();
}

  void UIWindow::putText(int x,int y,std::string Text, float i_xper, float i_yper) {
    int v_width, v_height;
    putTextS(x, y, Text,
	     v_width, v_height,
	     i_xper, i_yper);
  }
   
  void UIWindow::putImage(int x,int y,int nWidth,int nHeight,Texture *pImage) {
    if(pImage != NULL) {
      m_drawLib->drawImage(Vector2f(x+getAbsPosX(),y+getAbsPosY()),
					Vector2f(x+nWidth+getAbsPosX(),y+nHeight+getAbsPosY()),
					pImage,MAKE_COLOR(255,255,255,(int)(255*getOpacity()/100)), true);
    }
  }
  
  void UIWindow::setScissor(int x,int y,int nWidth,int nHeight) {
    /* This can be used for a simple 2-level scissor stack */
    m_drawLib->setClipRect(x+getAbsPosX(),y+getAbsPosY(),nWidth,nHeight);
  }
  
  void UIWindow::getScissor(int *px,int *py,int *pnWidth,int *pnHeight) {
    int x,y;
    m_drawLib->getClipRect(&x,&y,pnWidth,pnHeight);
    *px = x - getAbsPosX();
    *py = y - getAbsPosY();    
  }
  
  void UIWindow::putRect(int x,int y,int nWidth,int nHeight,Color c) {
    m_drawLib->drawBox(Vector2f(x+getAbsPosX(),y+getAbsPosY()),
                      Vector2f(x+nWidth+getAbsPosX(),y+nHeight+getAbsPosY()),
                      0,MAKE_COLOR(GET_RED(c),GET_GREEN(c),GET_BLUE(c),(int)(GET_ALPHA(c)*getOpacity()/100)),0);
  }

void UIWindow::putElem(int x,int y,int nWidth,int nHeight,UIElem Elem,bool bDisabled,bool bActive) {
  m_drawLib->setTexture(NULL, BLEND_MODE_NONE);
  Texture *vTexture = NULL;
    
  struct _ElemTable {
    UIElem E;
    int nX, nY, nWidth, nHeight;
  };
    
  static _ElemTable Table[] = {
    {UI_ELEM_LARGE_BUTTON_UP,0,0,207,57},
    {UI_ELEM_LARGE_BUTTON_DOWN,0,57,207,57},
    {UI_ELEM_SMALL_BUTTON_UP,0,114,115,57},
    {UI_ELEM_SMALL_BUTTON_DOWN,115,114,115,57},
    {UI_ELEM_CHECKBUTTON_UNCHECKED_UP,222,0,34,34},
    {UI_ELEM_CHECKBUTTON_UNCHECKED_DOWN,222,34,34,34},
    {UI_ELEM_CHECKBUTTON_CHECKED_UP,222,68,34,34},
    {UI_ELEM_CHECKBUTTON_CHECKED_DOWN,222,222,34,34},
    {UI_ELEM_RADIOBUTTON_UNCHECKED_UP,34,222,34,34},
    {UI_ELEM_RADIOBUTTON_UNCHECKED_DOWN,68,222,34,34},
    {UI_ELEM_RADIOBUTTON_CHECKED_UP,102,222,34,34},
    {UI_ELEM_RADIOBUTTON_CHECKED_DOWN,0,222,34,34},
    {UI_ELEM_SCROLLBUTTON_DOWN_UP,1,180,20,20},
    {UI_ELEM_SCROLLBUTTON_DOWN_DOWN,22,180,20,20},
    {UI_ELEM_SCROLLBUTTON_UP_UP,43,180,20,20},
    {UI_ELEM_SCROLLBUTTON_UP_DOWN,64,180,20,20},
    {UI_ELEM_SCROLLBUTTON_RIGHT_UP,85,180,20,20},
    {UI_ELEM_SCROLLBUTTON_RIGHT_DOWN,106,180,20,20},
    {UI_ELEM_SCROLLBUTTON_LEFT_UP,127,180,20,20},
    {UI_ELEM_SCROLLBUTTON_LEFT_DOWN,148,180,20,20},
    {UI_ELEM_FRAME_TL,169,180,8,8},
    {UI_ELEM_FRAME_TM,172,180,20,8},
    {UI_ELEM_FRAME_TR,187,180,8,8},
    {UI_ELEM_FRAME_ML,169,189,8,8},
    {UI_ELEM_FRAME_MM,178,184,8,20},
    {UI_ELEM_FRAME_MR,187,184,8,20},
    {UI_ELEM_FRAME_BL,169,198,8,8},
    {UI_ELEM_FRAME_BM,172,198,20,8},
    {UI_ELEM_FRAME_BR,187,198,8,8},
    {(UIElem)-1,-1,-1,-1,-1}
  };
        
  int nElem = 0;
  _ElemTable *p = NULL;
  while(Table[nElem].E != -1) {
    if(Table[nElem].E == Elem) {
      p = &Table[nElem];
      break;
    }
    nElem++;
  }
    
  if(p==NULL)
    return;
    
  int w = nWidth;
  int h = nHeight;

  if(w < 0)
    w = p->nWidth;
  if(h < 0)
    h = p->nHeight;

  float fX1 = ((float)p->nX) / 256.0f;
  float fY1 = ((float)p->nY) / 256.0f;
  float fX2 = ((float)p->nX+p->nWidth) / 256.0f;
  float fY2 = ((float)p->nY+p->nHeight) / 256.0f;

  Color c1,c2,c3,c4;
  c1=c2=c3=c4=MAKE_COLOR(255,255,255,(int)(255*getOpacity()/100));

  int cx = getAbsPosX() + x;
  int cy = getAbsPosY() + y;
  vTexture =NULL;
  /* Nice. Now we know what to draw */    
  if(bDisabled) {
    vTexture = UITexture::getMiscDisabledTexture();
  } else {
    vTexture = UITexture::getMiscTexture();
  }
  m_drawLib->setTexture(vTexture, BLEND_MODE_A);
  m_drawLib->startDraw(DRAW_MODE_POLYGON);

  m_drawLib->setColor(c1);
  m_drawLib->glTexCoord(fX1,fY1);
  m_drawLib->glVertexSP(cx,cy);

  m_drawLib->setColor(c2);
  m_drawLib->glTexCoord(fX2,fY1);        
  m_drawLib->glVertexSP(cx+w,cy);

  m_drawLib->setColor(c3);
  m_drawLib->glTexCoord(fX2,fY2);        
  m_drawLib->glVertexSP(cx+w,cy+h);

  m_drawLib->setColor(c4);
  m_drawLib->glTexCoord(fX1,fY2);
  m_drawLib->glVertexSP(cx,cy+h);

  m_drawLib->endDraw();
    
  /* Active? If so we want a nice blinking overlay */
  if(bActive && bDisabled == false) {
    float s = 160 + 76*sin(getApp()->getXMTime()*10);
    int n = (int)s;
    if(n<0)
      n=0;
    if(n>255)
      n=255; /* just to be sure, i'm lazy */    
    c1=c2=c3=c4=MAKE_COLOR(255,255,255,(int)(n*getOpacity()/100));

    vTexture = UITexture::getMiscActiveTexture();
    m_drawLib->setTexture(vTexture,BLEND_MODE_A);
    m_drawLib->startDraw(DRAW_MODE_POLYGON);
    m_drawLib->setColor(c1);
    m_drawLib->glTexCoord(fX1,fY1);
    m_drawLib->glVertexSP(cx,cy);
    m_drawLib->setColor(c2);
    m_drawLib->glTexCoord(fX2,fY1);        
    m_drawLib->glVertexSP(cx+w,cy);
    m_drawLib->setColor(c3);
    m_drawLib->glTexCoord(fX2,fY2);
    m_drawLib->glVertexSP(cx+w,cy+h);
    m_drawLib->setColor(c4);
    m_drawLib->glTexCoord(fX1,fY2);
    m_drawLib->glVertexSP(cx,cy+h);
    m_drawLib->endDraw();
  }
}
  
  /*
largebuttonup (0,0)  (207x57)
largebuttondown (0,57)  (207x57)
smallbuttonup (0,114) (115x57)
smallbuttondown (115,114) (115x57)
checkbutton unchecked up (222,0) (34x34)
checkbutton unchecked down (222,34) (34x34)
checkbutton checked up (222,68) (34x34)
checkbutton checked down (222,222) (34x34)
radbutton unchecked up (34,222) (34x34)
radbutton unchecked down (68,222) (34x34)
radbutton checked up (102,222) (34x34)
radbutton checked down (0,222) (34x34)
scrolldown up (1,180) (20x20)
scrolldown down (22,180) (20x20)
scrollup up (43,180) (20x20)
scrollup down (64,180) (20x20)
scrollright up (85,180) (20x20)
scrollright down (106,180) (20x20)
scrollleft up (127,180) (20x20)
scrollleft down (148,180) (20x20)

FRAME_TL (169,180) (8x8)
FRAME_TM (178,180) (8x8)
FRAME_TR (187,180) (8x8)
FRAME_ML (169,189) (8x8)
FRAME_MM (178,189) (8x8)
FRAME_MR (187,189) (8x8)
FRAME_BL (169,198) (8x8)
FRAME_BM (178,198) (8x8)
FRAME_BR (187,198) (8x8)
*/


  /*===========================================================================
  Root window
  ===========================================================================*/
UIRoot::UIRoot()
{
  m_pApp = GameApp::instance();
  m_bShowContextMenu = true;
  _InitWindow();
}

  void UIRoot::_ClipRect(UIRect *pRect,UIRect *pClipWith) {
    /* Find the area shared by the two */
    int nMinX = pRect->nX < pClipWith->nX ? pClipWith->nX : pRect->nX;
    int nMaxX = pRect->nX+pRect->nWidth > pClipWith->nX+pClipWith->nWidth ? 
                pClipWith->nX+pClipWith->nWidth : pRect->nX+pRect->nWidth;
    int nMinY = pRect->nY < pClipWith->nY ? pClipWith->nY : pRect->nY;
    int nMaxY = pRect->nY+pRect->nHeight > pClipWith->nY+pClipWith->nHeight ? 
                pClipWith->nY+pClipWith->nHeight : pRect->nY+pRect->nHeight;
    pRect->nX = nMinX;                
    pRect->nY = nMinY;                
    pRect->nWidth = nMaxX - nMinX;
    pRect->nHeight = nMaxY - nMinY;
  }
  
  void UIRoot::_RootPaint(int x,int y,UIWindow *pWindow,UIRect *pRect) {
    /* Hidden? */
    if(pWindow->isHidden())
      return;
  
    /* Clip rect to this window */
    UIRect WindowRect;
    WindowRect.nX = pWindow->getPosition().nX + x;
    WindowRect.nY = pWindow->getPosition().nY + y;
    WindowRect.nWidth = pWindow->getPosition().nWidth;
    WindowRect.nHeight = pWindow->getPosition().nHeight;  
    _ClipRect(&WindowRect,pRect);  
    
    /* Anything to paint? */
    if(WindowRect.nWidth > 0 && WindowRect.nHeight > 0) {
      /* Set scissor */
      m_drawLib->setClipRect(WindowRect.nX,WindowRect.nY,WindowRect.nWidth,WindowRect.nHeight);
        
      /* Invoke client painting code */
      pWindow->paint();

      /* Draw children */
      for(unsigned int i=0;i<pWindow->getChildren().size();i++) {
/*        printf("child: %s\n",pWindow->getChildren()[i]->getCaption().c_str());
        printf("Draw Rect: %d %d %d %d\n",WindowRect.nX,WindowRect.nY,WindowRect.nWidth,WindowRect.nHeight);*/
        _RootPaint(x+pWindow->getPosition().nX,y+pWindow->getPosition().nY,pWindow->getChildren()[i],&WindowRect);      
      }
      m_drawLib->setClipRect(NULL);
    }
  }
  
  void UIRoot::paint(void) {
    UIRect Screen;
    
    /* Clip to full screen */
    Screen.nX = 0;
    Screen.nY = 0;
    Screen.nWidth = m_drawLib->getDispWidth();
    Screen.nHeight = m_drawLib->getDispHeight();
      
    /* Draw root's children */
#ifdef ENABLE_OPENGL
    glEnable(GL_SCISSOR_TEST);
#endif

    for(unsigned int i=0;i<getChildren().size();i++)
      _RootPaint(0,0,getChildren()[i],&Screen);

#ifdef ENABLE_OPENGL
    glDisable(GL_SCISSOR_TEST);
#endif

    /* Context help? */
    if(m_bShowContextMenu && isDisabled() == false) {
      int nContextHelpHeight = 20;
      
      /* Shade out bottom of screen */
      m_drawLib->setBlendMode(BLEND_MODE_A);
      m_drawLib->startDraw(DRAW_MODE_POLYGON);
      //glColor4f(0,0,0,0);//fully transparent??
      m_drawLib->setColorRGBA(0,0,0,0);
      m_drawLib->glVertexSP(0,m_drawLib->getDispHeight()-nContextHelpHeight);
      m_drawLib->glVertexSP(m_drawLib->getDispWidth(),m_drawLib->getDispHeight()-nContextHelpHeight);
      //glColor4f(0,0,0,0.7);
      m_drawLib->setColorRGBA(0,0,0,255 * 7 / 100);
      m_drawLib->glVertexSP(m_drawLib->getDispWidth(),m_drawLib->getDispHeight());
      m_drawLib->glVertexSP(0,m_drawLib->getDispHeight());
      m_drawLib->endDraw();
        
      if(!m_CurrentContextHelp.empty()) {
        /* Print help string */
	setFont(m_drawLib->getFontSmall());
	setTextSolidColor(MAKE_COLOR(255,255,0,255));
	putText(m_drawLib->getDispWidth()  -5,
		m_drawLib->getDispHeight() -1,
		m_CurrentContextHelp, -1.0, -1.0);
      }
    }
  }
  
  void UIRoot::mouseLDown(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_LBUTTON_DOWN,x,y);
  }

  void UIRoot::mouseLDoubleClick(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_DOUBLE_CLICK,x,y);
  }
  
  void UIRoot::mouseLUp(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_LBUTTON_UP,x,y);
  }
  
  void UIRoot::mouseRDown(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_RBUTTON_DOWN,x,y);
  }
  
  void UIRoot::mouseRUp(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_RBUTTON_UP,x,y);
  }  
  
  void UIRoot::mouseHover(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_HOVER,x,y);
  } 

  void UIRoot::mouseWheelUp(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_WHEEL_UP,x,y);
  } 

  void UIRoot::mouseWheelDown(int x,int y) {
    _RootMouseEvent(this,UI_ROOT_MOUSE_WHEEL_DOWN,x,y);
  } 
  
  bool UIRoot::keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char) {
    if(!_RootKeyEvent(this,UI_ROOT_KEY_DOWN,nKey,mod, nChar, i_utf8Char)) {
      switch(nKey) {
        case SDLK_UP:
          activateUp();
          return true;
        case SDLK_DOWN:
          activateDown();
          return true;
        case SDLK_LEFT:
          activateLeft();
          return true;
        case SDLK_RIGHT:
          activateRight();
          return true;
        case SDLK_TAB:
          activateNext();
          return true;
        case SDLK_BACKSPACE:
          activatePrevious();
          return true;
      }
      return false;
    }
    return true;
  }
  
  bool UIRoot::keyUp(int nKey, SDLMod mod, const std::string& i_utf8Char) {
    return _RootKeyEvent(this,UI_ROOT_KEY_UP,nKey,mod, 0, i_utf8Char);
  }
  
  bool UIRoot::_RootKeyEvent(UIWindow *pWindow,UIRootKeyEvent Event,int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char) {
    /* Hidden or disabled? */
    if(pWindow->isHidden() || pWindow->isDisabled()) return false;

    /* First try if any children want it */
    for(unsigned int i=0;i<pWindow->getChildren().size();i++) {
      if(_RootKeyEvent(pWindow->getChildren()[i],Event,nKey,mod,nChar, i_utf8Char))
        return true;
    }

    /* Try this */
    if(pWindow != this && pWindow->isActive()) {
      bool b = false;
    
      switch(Event) {
      case UI_ROOT_KEY_DOWN: b=pWindow->keyDown(nKey, mod,nChar, i_utf8Char); break;
      case UI_ROOT_KEY_UP: b=pWindow->keyUp(nKey, mod, i_utf8Char); break;        
      }
      
      return b;
    }   
    else return false;
    
    /* Ok */
    return true;
  }
  
  bool UIRoot::_RootMouseEvent(UIWindow *pWindow,UIRootMouseEvent Event,int x,int y) {
    /* Hidden or disabled? */
    if(pWindow->isHidden() || pWindow->isDisabled()) return false;

    /* All root mouse events are handled the same... - first remap coords */
    int wx = x - pWindow->getPosition().nX;    
    int wy = y - pWindow->getPosition().nY;    
    
    /* Inside window? */
    if(wx >= 0 && wy >= 0 && wx < pWindow->getPosition().nWidth && wy < pWindow->getPosition().nHeight) {  
      /* Offer activation to this window first */
      if((Event == UI_ROOT_MOUSE_LBUTTON_UP || Event == UI_ROOT_MOUSE_RBUTTON_UP)
          && pWindow->offerActivation()) {
        /* Nice, acquire it. */
        deactivate(this); /* start by making sure nothing is activated */
        pWindow->setActive(true); /* then activate it */
      }      

      /* Recurse children */
      for(int i=(int)pWindow->getChildren().size()-1; i>=0; i--) {
        if(pWindow->getChildren()[i]->offerMouseEvent()) {
          if(_RootMouseEvent(pWindow->getChildren()[i],Event,wx,wy)) return true;
        }
      }
      
      /* Then it must be this */
      if(pWindow != this) {
        switch(Event) {
          case UI_ROOT_MOUSE_LBUTTON_DOWN: pWindow->mouseLDown(wx,wy); break;
          case UI_ROOT_MOUSE_RBUTTON_DOWN: pWindow->mouseRDown(wx,wy); break;
          case UI_ROOT_MOUSE_LBUTTON_UP: pWindow->mouseLUp(wx,wy); break;
          case UI_ROOT_MOUSE_RBUTTON_UP: pWindow->mouseRUp(wx,wy); break;
          case UI_ROOT_MOUSE_HOVER: {              
              /* Post mouse-hover event to window */
              pWindow->mouseHover(wx,wy); 
              
              /* By the way, does it want to offer context-help? (only 
                 if mouse have actually moved) */
              if(getApp()->haveMouseMoved()) {
                std::string SubCHelp = pWindow->subContextHelp(wx,wy);
                if(SubCHelp != "") m_CurrentContextHelp = SubCHelp;
                else m_CurrentContextHelp = pWindow->getContextHelp();
              }
              
              break;
            }
          case UI_ROOT_MOUSE_WHEEL_UP: pWindow->mouseWheelUp(wx,wy); break;
          case UI_ROOT_MOUSE_WHEEL_DOWN: pWindow->mouseWheelDown(wx,wy); break;
          case UI_ROOT_MOUSE_DOUBLE_CLICK: pWindow->mouseLDoubleClick(wx,wy); break;
        }
      } else {
	m_CurrentContextHelp = "";
      }
      
      /* Mkay */
      return true;
    }
    
    /* No go */
    return false;
  }  
  
  void UIRoot::deactivate(UIWindow *pWindow) {
    pWindow->setActive(false);
    for(unsigned int i=0;i<pWindow->getChildren().size();i++) {
      deactivate(pWindow->getChildren()[i]);
    }
  }

  /*===========================================================================
  Async. mouse state
  ===========================================================================*/
  bool UIWindow::isMouseLDown(void) {
    if(SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(1)) return true;
    return false;
  }
  
  bool UIWindow::isMouseRDown(void) {
    if(SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(2)) return true;
    return false;
  }

  /*===========================================================================
  Texture mangling
  ===========================================================================*/ 
  GameApp *UITexture::getApp(void) {
    return m_pApp;
  }
  
  Texture *UITexture::getMiscTexture(void) {
    return m_pUIElemTexture;
  }
  
  Texture *UITexture::getMiscDisabledTexture(void) {
    return m_pUIElemTextureD;
  }
  
  Texture *UITexture::getMiscActiveTexture(void) {
    return m_pUIElemTextureA;
  }
  
  void UITexture::setApp(GameApp *pApp) {
    Sprite *pSprite;

    pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "Misc");
    if(pSprite != NULL) {
      m_pUIElemTexture = pSprite->getTexture(false,true, FM_NEAREST);
    }

    pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "MiscDisabled");
    if(pSprite != NULL) {
      m_pUIElemTextureD = pSprite->getTexture(false,true, FM_NEAREST);
    }

    pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "MiscActive");
    if(pSprite != NULL) {
      m_pUIElemTextureA = pSprite->getTexture(false,true, FM_NEAREST);
    }
  }
  
  /* Static class data */  
  GameApp *UITexture::m_pApp = GameApp::instance();
  Texture *UITexture::m_pUIElemTexture = NULL;
  Texture *UITexture::m_pUIElemTextureD = NULL;
  Texture *UITexture::m_pUIElemTextureA = NULL;

  /*===========================================================================
  Activation maps
  ===========================================================================*/
  unsigned int UIRoot::_UpdateActivationMap(UIWindow *pWindow, UIRootActCandidate *pMap,
					    unsigned int nNum, unsigned int nMax) {
    if(nNum >= nMax){
      return nNum;
    }
  
    /* Find all windows which accepts activations, and list them in a nice structure */
    if(!pWindow->isDisabled() && pWindow->offerActivation() && pWindow->canGetFocus()) {
      pMap[nNum].x = pWindow->getAbsPosX(); 
      pMap[nNum].nWidth= pWindow->getPosition().nWidth;
      pMap[nNum].y = pWindow->getAbsPosY();
      pMap[nNum].nHeight = pWindow->getPosition().nHeight;
      pMap[nNum].pWindow = pWindow;
      nNum++;
    }
    
    /* Recurse */
    for(unsigned int i=0;i<pWindow->getChildren().size();i++) {
      if(!pWindow->isHidden() && !pWindow->isDisabled()){
        nNum = _UpdateActivationMap(pWindow->getChildren()[i],pMap,nNum,nMax);
      }
    }
    
    /* Return number of candidates */
    return nNum;
  }

  int UIRoot::_GetActiveIdx(UIRootActCandidate *pMap, unsigned int nNum) {
    for(unsigned int i=0; i<nNum; i++){
      if(pMap[i].pWindow->isActive()){
        return i;
      }
    }
    return -1;
  }

void UIRoot::_ActivateByVector(int dx, int dy) {
  /* Update the activation map */
  UIRootActCandidate Map[128];
  unsigned int nNum = _UpdateActivationMap(this, Map, 0, 128);
    
  if(nNum > 0) {
    /* Find the currently active window */
    int nActive = _GetActiveIdx(Map, nNum);

    /* No active? */
    if(nActive < 0) {
      /* Simply activate the first */        
      deactivate(this);
      if(dx<0 || dy<0)
	Map[0].pWindow->setActive(true);
      else if(dx>0 || dy>0)
	Map[nNum-1].pWindow->setActive(true);
      return;
    }
    else {
      /* Okay, determine where to go from here */
      /* Calculate a "best estimate" rating for each candidate, and select the best */
      int nBest          = -1;
	  int hBest    = -1;
      int vBest    = -1;
	  int hCurrent = -1;
      int vCurrent = -1;
      unsigned int uActive = nActive;
	  int ax = Map[uActive].x + Map[uActive].nWidth / 2;
      int ay = Map[uActive].y + Map[uActive].nHeight / 2;

      for(unsigned int i=0; i<nNum; i++) {
	if(i != uActive) {
	  /* Calculate rating */
      int vx = Map[i].x - ax;
	  int vy = Map[i].y - ay;
	  int vx2 = Map[i].x + Map[i].nWidth  - ax;
	  int vy2 = Map[i].y + Map[i].nHeight - ay;

      /* Avoid crazy candidates */
      int mx= (vx+vx2)/2;
      int my= (vy+vy2)/2;
      if( (dx>0 && mx<=0) || (dx<0 && mx>=0) || (dy>0 && my<=0) || (dy<0 && my>=0) ){
          continue;
      }
      
      vx = Map[i].x - ax;
	  vy = Map[i].y - ay;
	  vx2 = Map[i].x + Map[i].nWidth  - ax;
	  vy2 = Map[i].y + Map[i].nHeight - ay;

      
      
      int xx1 = vx, yy1 = vy;
      /* Distance with corners */
      if(vx>0 && vy2<0){
        yy1 = vy2;
      }
      else if(vx2<0 && vy>0){
          xx1 = vx2;
      }
      else if(vx2<0 && vy2<0) {
          xx1 = vx2;
          yy1= vy2;
      }
      /* Distance with edges */
      else if(vx<=0 && vx2>=0 && vy >= 0){
        xx1 = 0;
        yy1 = vy;
      }
      else if(vx<=0 && vx2>=0 && vy2 <= 0){
        xx1 = 0;
        yy1 = vy2;
      }
      else if(vy<=0 && vy2>=0 && vx >= 0){
        xx1 = vx;
        yy1 = 0;
      }
      else if(vy<=0 && vy2>=0 && vx2 <= 0){
        xx1 = vx2;
        yy1 = 0;
      }

      if(dx != 0 && dy == 0){
          if(xx1 != 0){
            hCurrent = abs(xx1);
            vCurrent = abs(yy1);
          }
      }

      if(dx == 0 && dy != 0){
          if(yy1 != 0){
            hCurrent = abs(yy1);
            vCurrent = abs(xx1);
          }
      }

            
	  /* Good? */
	  if( (vCurrent<=vBest && vBest>=0) || vBest < 0 ) {
	    if( (vCurrent < vBest) || (hCurrent < hBest) || vBest <0 ) {
            vBest = vCurrent;
            hBest = hCurrent;
            nBest = i;
        }
	  }
	}
      }
        
      /* Did we get something? */
      if(nBest>=0) {
	deactivate(this);
	Map[nBest].pWindow->setActive(true);          
          
	m_CurrentContextHelp = Map[nBest].pWindow->getContextHelp();
      }
      else {
	deactivate(this);
	if(dx<0 || dy<0) {
	  Map[0].pWindow->setActive(true);
	  m_CurrentContextHelp = Map[0].pWindow->getContextHelp();
	}
	else if(dx>0 || dy>0) {
	  Map[nNum-1].pWindow->setActive(true);
	  m_CurrentContextHelp = Map[nNum-1].pWindow->getContextHelp();
	}
      }
    }
  }
}

void UIRoot::_ActivateByStep(int step) {
  /* Update the activation map */
  UIRootActCandidate Map[128];
  unsigned int nNum = _UpdateActivationMap(this, Map, 0, 128);

  if(nNum > 0) {
    /* Find the currently active window */
    int nActive = _GetActiveIdx(Map, nNum);

    /* No active? */
    if(nActive < 0) {
      /* Simply activate the first */        
      deactivate(this);
      if(step > 0){
        Map[0].pWindow->setActive(true);
      }
      else if(step < 0) {
        Map[nNum-1].pWindow->setActive(true);
      }
      return;
    }
    else {
        unsigned int nBest = nActive + step;
        if(nBest < 0 ){
            nBest += nNum;
        }
        if(nBest >= nNum){
            nBest -= nNum;
        }
        deactivate(this);
        Map[nBest].pWindow->setActive(true);          
        m_CurrentContextHelp = Map[nBest].pWindow->getContextHelp();
        return;
    }
  }
}

  void UIRoot::activateUp(void) {
    _ActivateByVector(0,-1);
  }
  
  void UIRoot::activateDown(void) {
    _ActivateByVector(0,1);
  }
  
  void UIRoot::activateLeft(void) {
    _ActivateByVector(-1,0);
  }
  
  void UIRoot::activateRight(void) {
    _ActivateByVector(1,0);
  }

  void UIRoot::activateNext(void) {
    _ActivateByStep(1);
  }

  void UIRoot::activatePrevious(void) {
    _ActivateByStep(-1);
  }

void UIRoot::dispatchMouseHover() {
  int nX,nY;
  GameApp::getMousePos(&nX,&nY);
  mouseHover(nX,nY);
}

UIProgressBar::UIProgressBar(UIWindow *pParent,
			     int x, int y,
			     int nWidth, int nHeight)
{
  initW(pParent, x, y, "", nWidth, nHeight);
  m_progress = 0;
  m_curOp    = "";
}

UIProgressBar::~UIProgressBar()
{
}

void UIProgressBar::paint()
{
  int width  = getPosition().nWidth;
  int height = getPosition().nHeight;
  // 1.paint it black
  putRect(0, 0, width, height, MAKE_COLOR(0,0,0,255));
  // 2.paint progress in red
  putRect(0, 0, width * m_progress / 100, height, MAKE_COLOR(255,0,0,255));
  // 3.write curOp left centered
  setFont(m_drawLib->getFontSmall());
  setTextSolidColor(MAKE_COLOR(255,255,255,128));
  putText(0, 0, m_curOp, 0.0, 0.0);
}

void UIProgressBar::setProgress(int progress)
{
  m_progress = progress;
}

void UIProgressBar::setCurrentOperation(std::string curOp)
{
  m_curOp = curOp;
}
