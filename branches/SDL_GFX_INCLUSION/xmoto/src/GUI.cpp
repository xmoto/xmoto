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
 *  In-game graphical user interface
 */
#include "VXml.h"
#include "GUI.h"
#include "GameText.h"

namespace vapp {

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
  
  /*===========================================================================
  Base window init/shutdown
  ===========================================================================*/
  bool UIWindow::haveChildW(UIWindow *pWindow) {
    for(int i=0;i<m_Children.size();i++)
      if(m_Children[i] == pWindow) return true;
    return false;
  }
  
  void UIWindow::addChildW(UIWindow *pWindow) {    
    if(!haveChildW(pWindow))
      m_Children.push_back(pWindow);
  }    
  
  void UIWindow::removeChildW(UIWindow *pWindow) {
    for(int i=0;i<m_Children.size();i++) {
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
    setApp(getParent()->getApp());    
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
  
  UIRoot *UIWindow::getRoot(void) {
    UIRoot *pRoot = NULL;
    for(UIWindow *p=this;p!=NULL;p=p->getParent()) {
      if(p->getParent() == NULL) {
        return reinterpret_cast<UIRoot *>(p);
      }
    }    
    
    return NULL;
  }
  
  void UIWindow::enableChildren(bool bState) {
    for(int i=0;i<m_Children.size();i++)
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
    
    for(int i=0;i<getChildren().size();i++) {
      if(getChildren()[i]->getID() == X) {
        if(XRem != "") return getChildren()[i]->getChild(XRem);
        else return getChildren()[i];
      }
    }
    return NULL;
  }

  void UIWindow::getTextExt(std::string Text,int *pnMinX,int *pnMinY,int *pnMaxX,int *pnMaxY) {
    if(m_pCurFont != NULL) {
      UITextDraw::getTextExt(m_pCurFont,Text,pnMinX,pnMinY,pnMaxX,pnMaxY);
    }    
  }
  
  void UIWindow::getTextSize(std::string Text,int *pnX,int *pnY) {
    int x1=0,y1=0,x2=0,y2=0;
    getTextExt(Text,&x1,&y1,&x2,&y2);
    if(pnX) *pnX = x2 - x1;
    if(pnY) *pnY = y2 - y1;
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
  bool UIQueryKeyBox::keyDown(int nKey,int nChar) {
    //    MessageBox(NULL,"HELLO",NULL,MB_OK);
    return false;
  }
  
  /*===========================================================================
  Message boxes
  ===========================================================================*/  
  UIMsgBoxButton UIMsgBox::getClicked(void) {
    /* Go through buttons... anything clicked? */
    for(int i=0;i<m_nNumButtons;i++) {
      if(m_pButtons[i]->isClicked()) {  
        if(m_pButtons[i]->getCaption() == GAMETEXT_OK) return UI_MSGBOX_OK;
        if(m_pButtons[i]->getCaption() == GAMETEXT_CANCEL) return UI_MSGBOX_CANCEL;
        if(m_pButtons[i]->getCaption() == GAMETEXT_YES) return UI_MSGBOX_YES;
        if(m_pButtons[i]->getCaption() == GAMETEXT_NO) return UI_MSGBOX_NO;
        if(m_pButtons[i]->getCaption() == GAMETEXT_YES_FOR_ALL) return UI_MSGBOX_YES_FOR_ALL;
      }
    }
    return UI_MSGBOX_NOTHING;
  }
  
  bool UIMsgBox::setClicked(std::string Text) {
    bool b=false;
    for(int i=0;i<m_nNumButtons;i++) {
      if(m_pButtons[i]->getCaption() == Text) {
        m_pButtons[i]->setClicked(true);
        b = true;
      }
    }
    return b;
  }
  
  void UIMsgBox::_ReEnableSiblings(void) {
    UIWindow *pParent = getParent();
    for(int i=0;i<pParent->getChildren().size()-1;i++) {
      pParent->getChildren()[i]->enableWindow(m_SiblingStates[i]);
    }
  }

  void UIMsgBox::paint(void) {
    /* Should the OK button be disabled? (if any) */
    if(m_bTextInput && m_TextInput.empty()) {
      for(int i=0;i<m_nNumButtons;i++) {
        if(m_pButtons[i]->getCaption() == GAMETEXT_OK) m_pButtons[i]->enableWindow(false);
      }
    }
    else {
      for(int i=0;i<m_nNumButtons;i++) {
        if(m_pButtons[i]->getCaption() == GAMETEXT_OK) m_pButtons[i]->enableWindow(true);
      }
    }   
  
    /* First draw frame */
    UIFrame::paint();
    
    /* Should we do some text? */
    if(m_bTextInput && m_pTextInputFont!=NULL) {      
      setFont(m_pTextInputFont);
      putText(16,120,m_TextInput + std::string("|"));
    }
  }
  
  bool UIMsgBox::keyDown(int nKey,int nChar) {
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
    int nNumButtons=0;
    
    if(Buttons & UI_MSGBOX_OK) nNumButtons++;
    if(Buttons & UI_MSGBOX_CANCEL) nNumButtons++;
    if(Buttons & UI_MSGBOX_YES) nNumButtons++;
    if(Buttons & UI_MSGBOX_NO) nNumButtons++;
    if(Buttons & UI_MSGBOX_YES_FOR_ALL) nNumButtons++;

    /* Determine size of contents */
    int x1,y1,x2,y2;    
    getTextExt(Text,&x1,&y1,&x2,&y2);
    int nButtonSize = 57;
    int w = ((x2 - x1) > nNumButtons*115 ? (x2 - x1) : nNumButtons*115) + 16 + 100;
    int h = y2 - y1 + nButtonSize + 24 + 100;
    
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
      for(int i=0;i<getChildren().size()-1;i++) {      
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
  void UIWindow::putText(int x,int y,std::string Text,bool bRotated) {
    /* Draw text at location */
    if(m_pCurFont != NULL) {
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

      UIFont *v_font = getFont();
      if(v_font != NULL) {
	UITextDraw::printRawGrad(v_font,getAbsPosX()+x+1,getAbsPosY()+y+1,
				 Text,MAKE_COLOR(0,0,0,255),MAKE_COLOR(0,0,0,255),MAKE_COLOR(0,0,0,255),MAKE_COLOR(0,0,0,255),bRotated);
	UITextDraw::printRawGrad(v_font,getAbsPosX()+x,getAbsPosY()+y,
				 Text,c0,c1,c2,c3,bRotated);
      }
    }
  }
   
  void UIWindow::putImage(int x,int y,int nWidth,int nHeight,Texture *pImage) {
    if(pImage != NULL) {
      getApp()->getDrawLib()->drawImage(Vector2f(x+getAbsPosX(),y+getAbsPosY()),
                          Vector2f(x+nWidth+getAbsPosX(),y+nHeight+getAbsPosY()),
                          pImage,MAKE_COLOR(255,255,255,(int)(255*getOpacity()/100)));
    }
  }
  
  void UIWindow::setScissor(int x,int y,int nWidth,int nHeight) {
    /* This can be used for a simple 2-level scissor stack */
    getApp()->scissorGraphics(x+getAbsPosX(),y+getAbsPosY(),nWidth,nHeight);
  }
  
  void UIWindow::getScissor(int *px,int *py,int *pnWidth,int *pnHeight) {
    int x,y;
    getApp()->getScissorGraphics(&x,&y,pnWidth,pnHeight);
    *px = x - getAbsPosX();
    *py = y - getAbsPosY();    
  }
  
  void UIWindow::putRect(int x,int y,int nWidth,int nHeight,Color c) {
    getApp()->getDrawLib()->drawBox(Vector2f(x+getAbsPosX(),y+getAbsPosY()),
                      Vector2f(x+nWidth+getAbsPosX(),y+nHeight+getAbsPosY()),
                      0,MAKE_COLOR(GET_RED(c),GET_GREEN(c),GET_BLUE(c),(int)(GET_ALPHA(c)*getOpacity()/100)),0);
  }

  void UIWindow::putElem(int x,int y,int nWidth,int nHeight,UIElem Elem,bool bDisabled,bool bActive) {
    Texture *vTexture = NULL;

    struct _ElemTable {
      UIElem E; int nX,nY,nWidth,nHeight;
    };
    
    static _ElemTable Table[] = {
      UI_ELEM_LARGE_BUTTON_UP,0,0,207,57,
      UI_ELEM_LARGE_BUTTON_DOWN,0,57,207,57,
      UI_ELEM_SMALL_BUTTON_UP,0,114,115,57,
      UI_ELEM_SMALL_BUTTON_DOWN,115,114,115,57,
      UI_ELEM_CHECKBUTTON_UNCHECKED_UP,222,0,34,34,
      UI_ELEM_CHECKBUTTON_UNCHECKED_DOWN,222,34,34,34,
      UI_ELEM_CHECKBUTTON_CHECKED_UP,222,68,34,34,
      UI_ELEM_CHECKBUTTON_CHECKED_DOWN,222,222,34,34,
      UI_ELEM_RADIOBUTTON_UNCHECKED_UP,34,222,34,34,
      UI_ELEM_RADIOBUTTON_UNCHECKED_DOWN,68,222,34,34,
      UI_ELEM_RADIOBUTTON_CHECKED_UP,102,222,34,34,
      UI_ELEM_RADIOBUTTON_CHECKED_DOWN,0,222,34,34,
      UI_ELEM_SCROLLBUTTON_DOWN_UP,1,180,20,20,
      UI_ELEM_SCROLLBUTTON_DOWN_DOWN,22,180,20,20,
      UI_ELEM_SCROLLBUTTON_UP_UP,43,180,20,20,
      UI_ELEM_SCROLLBUTTON_UP_DOWN,64,180,20,20,
      UI_ELEM_SCROLLBUTTON_RIGHT_UP,85,180,20,20,
      UI_ELEM_SCROLLBUTTON_RIGHT_DOWN,106,180,20,20,
      UI_ELEM_SCROLLBUTTON_LEFT_UP,127,180,20,20,
      UI_ELEM_SCROLLBUTTON_LEFT_DOWN,148,180,20,20,
      UI_ELEM_FRAME_TL,169,180,8,8,
      UI_ELEM_FRAME_TM,178,180,8,8,
      UI_ELEM_FRAME_TR,187,180,8,8,
      UI_ELEM_FRAME_ML,169,189,8,8,
      UI_ELEM_FRAME_MM,178,189,8,8,
      UI_ELEM_FRAME_MR,187,189,8,8,
      UI_ELEM_FRAME_BL,169,198,8,8,
      UI_ELEM_FRAME_BM,178,198,8,8,
      UI_ELEM_FRAME_BR,187,198,8,8,
      (UIElem)-1
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
    
    if(p==NULL) return;
    
    int w = nWidth;
    int h = nHeight;
    if(w < 0) w = p->nWidth;
    if(h < 0) h = p->nHeight;
    
    float fX1 = ((float)p->nX) / 256.0f;
    float fY1 = ((float)p->nY) / 256.0f;
    float fX2 = ((float)p->nX+p->nWidth) / 256.0f;
    float fY2 = ((float)p->nY+p->nHeight) / 256.0f;
    
    Color c1,c2,c3,c4;
    c1=c2=c3=c4=MAKE_COLOR(255,255,255,(int)(255*getOpacity()/100));
    
    int cx = getAbsPosX() + x;
    int cy = getAbsPosY() + y;
    /* Nice. Now we know what to draw */    
    if(bDisabled) {
      vTexture = UITexture::getMiscDisabledTexture();
    } else {
      vTexture = UITexture::getMiscTexture();
    }
    getApp()->getDrawLib()->setTexture(vTexture,BLEND_MODE_A);
    getApp()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);

    getApp()->getDrawLib()->setColor(c1);
    getApp()->getDrawLib()->glTexCoord(fX1,fY1);
    getApp()->getDrawLib()->glVertexSP(cx,cy);

    getApp()->getDrawLib()->setColor(c2);
    getApp()->getDrawLib()->glTexCoord(fX2,fY1);        
    getApp()->getDrawLib()->glVertexSP(cx+w,cy);

    getApp()->getDrawLib()->setColor(c3);
    getApp()->getDrawLib()->glTexCoord(fX2,fY2);        
    getApp()->getDrawLib()->glVertexSP(cx+w,cy+h);

    getApp()->getDrawLib()->setColor(c4);
    getApp()->getDrawLib()->glTexCoord(fX1,fY2);
    getApp()->getDrawLib()->glVertexSP(cx,cy+h);

    getApp()->getDrawLib()->endDraw();
    
    /* Active? If so we want a nice blinking overlay */
    if(bActive) {
      float s = 160 + 76*sin(getApp()->getRealTime()*10);
      int n = (int)s;
      if(n<0) n=0;
      if(n>255) n=255; /* just to be sure, i'm lazy */    
      c1=c2=c3=c4=MAKE_COLOR(255,255,255,(int)(n*getOpacity()/100));

      vTexture = UITexture::getMiscActiveTexture();
      getApp()->getDrawLib()->setTexture(vTexture,BLEND_MODE_A);
      getApp()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
      getApp()->getDrawLib()->setColor(c1);
      getApp()->getDrawLib()->glTexCoord(fX1,fY1);
      getApp()->getDrawLib()->glVertexSP(cx,cy);
      getApp()->getDrawLib()->setColor(c2);
      getApp()->getDrawLib()->glTexCoord(fX2,fY1);        
      getApp()->getDrawLib()->glVertexSP(cx+w,cy);
      getApp()->getDrawLib()->setColor(c3);
      getApp()->getDrawLib()->glTexCoord(fX2,fY2);
      getApp()->getDrawLib()->glVertexSP(cx+w,cy+h);
      getApp()->getDrawLib()->setColor(c4);
      getApp()->getDrawLib()->glTexCoord(fX1,fY2);
      getApp()->getDrawLib()->glVertexSP(cx,cy+h);
      getApp()->getDrawLib()->endDraw();
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
    if(pWindow->isHidden()) return;
  
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
      getApp()->scissorGraphics(WindowRect.nX,WindowRect.nY,WindowRect.nWidth,WindowRect.nHeight);
        
      /* Invoke client painting code */
      pWindow->paint();

      /* Draw children */
      for(int i=0;i<pWindow->getChildren().size();i++) {
/*        printf("child: %s\n",pWindow->getChildren()[i]->getCaption().c_str());
        printf("Draw Rect: %d %d %d %d\n",WindowRect.nX,WindowRect.nY,WindowRect.nWidth,WindowRect.nHeight);*/
        _RootPaint(x+pWindow->getPosition().nX,y+pWindow->getPosition().nY,pWindow->getChildren()[i],&WindowRect);      
      }
    }
  }
  
  void UIRoot::paint(void) {
    UIRect Screen;
    
    /* Clip to full screen */
    Screen.nX = 0;
    Screen.nY = 0;
    Screen.nWidth = getApp()->getDrawLib()->getDispWidth();
    Screen.nHeight = getApp()->getDrawLib()->getDispHeight();
      
    /* Draw root's children */
#ifdef ENABLE_OPENGL
    glEnable(GL_SCISSOR_TEST);
#endif

    for(int i=0;i<getChildren().size();i++)
      _RootPaint(0,0,getChildren()[i],&Screen);

#ifdef ENABLE_OPENGL
    glDisable(GL_SCISSOR_TEST);
#endif

    /* Context help? */
    if(m_bShowContextMenu) {
      int nContextHelpHeight = 20;
      
      /* Shade out bottom of screen */
      getApp()->getDrawLib()->setBlendMode(BLEND_MODE_A);
      getApp()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
      //glColor4f(0,0,0,0);//fully transparent??
      getApp()->getDrawLib()->setColorRGBA(0,0,0,0);
      getApp()->getDrawLib()->glVertexSP(0,getApp()->getDrawLib()->getDispHeight()-nContextHelpHeight);
      getApp()->getDrawLib()->glVertexSP(getApp()->getDrawLib()->getDispWidth(),getApp()->getDrawLib()->getDispHeight()-nContextHelpHeight);
      //glColor4f(0,0,0,0.7);
      getApp()->getDrawLib()->setColorRGBA(0,0,0,255 * 7 / 100);
      getApp()->getDrawLib()->glVertexSP(getApp()->getDrawLib()->getDispWidth(),getApp()->getDrawLib()->getDispHeight());
      getApp()->getDrawLib()->glVertexSP(0,getApp()->getDrawLib()->getDispHeight());
      getApp()->getDrawLib()->endDraw();
        
      if(!m_CurrentContextHelp.empty()) {
        /* Print help string */
        int nX1,nY1,nX2,nY2;
        getTextExt(m_CurrentContextHelp,&nX1,&nY1,&nX2,&nY2);
        
	UIFont *v_font = getFont();

	if(v_font != NULL) {
	  UITextDraw::printRaw(v_font,getApp()->getDrawLib()->getDispWidth()-(nX2-nY1),getApp()->getDrawLib()->getDispHeight()-5,
			       m_CurrentContextHelp,MAKE_COLOR(255,255,0,255));
	}
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
  
  bool UIRoot::keyDown(int nKey,int nChar) {
    if(!_RootKeyEvent(this,UI_ROOT_KEY_DOWN,nKey,nChar)) {
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
      }
      
      return false;
    }
    return true;
  }
  
  bool UIRoot::keyUp(int nKey) {
    return _RootKeyEvent(this,UI_ROOT_KEY_UP,nKey,0);   
  }
  
  bool UIRoot::_RootKeyEvent(UIWindow *pWindow,UIRootKeyEvent Event,int nKey,int nChar) {
    /* Hidden or disabled? */
    if(pWindow->isHidden() || pWindow->isDisabled()) return false;
        
    /* First try if any children want it */
    for(int i=0;i<pWindow->getChildren().size();i++) {
      if(_RootKeyEvent(pWindow->getChildren()[i],Event,nKey,nChar))
        return true;
    } 
    
    /* Try this */
    if(pWindow != this && pWindow->isActive()) {
      bool b = false;
    
      switch(Event) {
        case UI_ROOT_KEY_DOWN: b=pWindow->keyDown(nKey,nChar); break;
        case UI_ROOT_KEY_UP: b=pWindow->keyUp(nKey); break;        
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
      for(int i=pWindow->getChildren().size()-1;i>=0;i--) {
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
      }
      
      /* Mkay */
      return true;
    }
    
    /* No go */
    return false;
  }  
  
  void UIRoot::deactivate(UIWindow *pWindow) {
    pWindow->setActive(false);
    for(int i=0;i<pWindow->getChildren().size();i++) {
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
  Text drawing
  ===========================================================================*/
  void UITextDraw::initTextDrawing(App *pApp) {
    if(m_nRefCount == 0) {
      m_pApp = pApp;
    
      /* Load font definitions from XML source */
      XMLDocument FontDoc;      
      FontDoc.readFromFile("fonts.dat");
      TiXmlDocument *pFontData = FontDoc.getLowLevelAccess();      
      if(pFontData == NULL)
        Log("** Warning ** : failed to load fonts from 'fonts.dat'");
      else {        
        TiXmlElement *pFontDataElem = pFontData->FirstChildElement("fontdata");
        if(pFontDataElem == NULL)
          Log("** Warning ** : no fonts found in 'fonts.dat'");
        else {
          for(TiXmlElement *pFontMap=pFontDataElem->FirstChildElement("fontmap");
              pFontMap!=NULL;pFontMap=pFontMap->NextSiblingElement("fontmap")) {
            /* Nice, got a font-map. Get name and image file name */
            const char *pc;
            pc = pFontMap->Attribute("name");
            if(pc==NULL)
              Log("** Warning ** : ignoring unnamed font");
            else {
              std::string Name = pc;
              std::string Image;
              
              pc = pFontMap->Attribute("image");
              if(pc==NULL) {
                /* Guess a image file name */
                Image = Name + ".png";
              }
              else
                Image = pc;
                
              /* Try loading texture */
              Texture *pTexture = NULL;
	      Sprite *pSprite   = getApp()->getTheme()->getSprite(SPRITE_TYPE_FONT, Name);
	      if(pSprite != NULL) {
	      	pTexture = pSprite->getTexture(false, true, FM_NEAREST);
	      }


              if(pTexture == NULL)
                Log("** Warning ** : font texture '%s' missing",Image.c_str());
              else {                  
                /* Got an empty font */
                UIFont *pFont = new UIFont;
                pFont->Name = Name;
                pFont->pTexture = pTexture;
                m_Fonts.push_back(pFont);
                
                int nTW,nTH;
                
                nTW = pTexture->nWidth;
                nTH = pTexture->nHeight;
                
                /* Fill it */
                for(TiXmlElement *pCharElem=pFontMap->FirstChildElement("char");
                    pCharElem!=NULL;pCharElem=pCharElem->NextSiblingElement("char")) {
                  /* Get info */
                  const char *pcI = pCharElem->Attribute("i");
                  const char *pcX = pCharElem->Attribute("x");
                  const char *pcY = pCharElem->Attribute("y");
                  const char *pcW = pCharElem->Attribute("w");
                  const char *pcH = pCharElem->Attribute("h");
                  const char *pcINCX = pCharElem->Attribute("incx");
                  const char *pcINCY = pCharElem->Attribute("incy");
                  const char *pcOX = pCharElem->Attribute("ox");
                  const char *pcOY = pCharElem->Attribute("oy");
                  
                  //printf("%s %s %s %s %s %s %s %s %s\n",
                  //   pcI,pcX,pcY,pcW,pcH,pcINCX,pcINCY,pcOX,pcOY);
                  
                  if(pcI && pcX && pcY && pcW && pcH && pcINCX && pcINCY && pcOX && pcOY) {
                    int nChar = atoi(pcI);
                    if(nChar >= 0 && nChar < 256) {
                      pFont->Chars[nChar].bAvail = true;
                      pFont->Chars[nChar].nWidth = atoi(pcW);
                      pFont->Chars[nChar].nHeight = atoi(pcH);
                      pFont->Chars[nChar].nIncX = atoi(pcINCX);
                      pFont->Chars[nChar].nIncY = atoi(pcINCY);
                      pFont->Chars[nChar].nOffsetX = atoi(pcOX);
                      pFont->Chars[nChar].nOffsetY = atoi(pcOY);

                      pFont->Chars[nChar].fX1 = (float)atoi(pcX)/(float)nTW;                      
                      pFont->Chars[nChar].fY1 = (float)atoi(pcY)/(float)nTH;                      
                      pFont->Chars[nChar].fX2 = (float)(atoi(pcX)+atoi(pcW))/(float)nTW;                      
                      pFont->Chars[nChar].fY2 = (float)(atoi(pcY)+atoi(pcH))/(float)nTH;                      
                    }
                  }
                  else
                    Log("** Warning ** : character in font '%s' is not fully defined",Name.c_str());
                }
              }
            }
          }
        }
      }
      
      Log(" %d font%s loaded",m_Fonts.size(),m_Fonts.size()==1?"":"s");
    }
    m_nRefCount++;
  }
  
  void UITextDraw::uninitTextDrawing(void) {
    m_nRefCount--;    
    if(m_nRefCount == 0) {        
      for(int i=0;i<m_Fonts.size();i++)
        delete m_Fonts[i];
    }
  }
  
  UIFont *UITextDraw::getFont(std::string Name) {
    for(int i=0;i<m_Fonts.size();i++)
      if(m_Fonts[i]->Name == Name) return m_Fonts[i];
    return NULL;
  }
  
  void UITextDraw::printRaw(UIFont *pFont,int x,int y,std::string Text,Color c) {  
    /* Draw text string */
    int cx=x,cy=y;
    
    for(int i=0;i<Text.size();i++) {
      int nChar = Text[i];
      if(nChar == '\n') {
        cy += (pFont->Chars['|'].nHeight*3)/2;
        cx = x;
      }
      else if(pFont->Chars[nChar].bAvail) {
        getApp()->getDrawLib()->setTexture(pFont->pTexture,BLEND_MODE_A);
        getApp()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	getApp()->getDrawLib()->setColor(c);
        getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY1);
        getApp()->getDrawLib()->glVertexSP((cx+pFont->Chars[nChar].nOffsetX),(cy-pFont->Chars[nChar].nOffsetY));
        getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY1);        
        getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nWidth+pFont->Chars[nChar].nOffsetX,cy-pFont->Chars[nChar].nOffsetY);
        getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY2);
        getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nWidth+pFont->Chars[nChar].nOffsetX,cy+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY);
        getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY2);
        getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nOffsetX,cy+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY);
        getApp()->getDrawLib()->endDraw();
        cx += pFont->Chars[nChar].nIncX;
      }
      else
        cx += pFont->Chars['-'].nIncX;
    }
    
  }

  void UITextDraw::printRawGrad(UIFont *pFont,int x,int y,std::string Text,Color c1,Color c2,Color c3,Color c4,bool bRotated) {  
    /* Draw text string */
    int cx=x,cy=y;
    

    for(int i=0;i<Text.size();i++) {
      int nChar = Text[i];
      if(nChar == '\n') {
        if(bRotated) {
          cy += (pFont->Chars['|'].nHeight*3)/2;
          cx = x;
        }
        else {
          cy += (pFont->Chars['|'].nHeight*3)/2;
          cx = x;
        }
      }
      else if(pFont->Chars[nChar].bAvail) {
        if(bRotated) {
          //glBegin(GL_POLYGON);
          //glColor4ub(GET_RED(c1),GET_GREEN(c1),GET_BLUE(c1),GET_ALPHA(c1));
          //glTexCoord2f(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY1);
          //getApp()->glVertexSP((cx-pFont->Chars[nChar].nOffsetY),(cy-pFont->Chars[nChar].nOffsetX));
          //glColor4ub(GET_RED(c2),GET_GREEN(c2),GET_BLUE(c2),GET_ALPHA(c2));
          //glTexCoord2f(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY1);        
          //getApp()->glVertexSP(cx-pFont->Chars[nChar].nOffsetY,cy-pFont->Chars[nChar].nOffsetX-pFont->Chars[nChar].nWidth);
          //glColor4ub(GET_RED(c3),GET_GREEN(c3),GET_BLUE(c3),GET_ALPHA(c3));
          //glTexCoord2f(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY2);
          //getApp()->glVertexSP(cx+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY,cy-pFont->Chars[nChar].nOffsetX-pFont->Chars[nChar].nWidth);
          //glColor4ub(GET_RED(c4),GET_GREEN(c4),GET_BLUE(c4),GET_ALPHA(c4));
          //glTexCoord2f(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY2);
          //getApp()->glVertexSP(cx+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY,(cy-pFont->Chars[nChar].nOffsetX));
          //glEnd();

	  getApp()->getDrawLib()->setTexture(pFont->pTexture,BLEND_MODE_A);
	  getApp()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	  getApp()->getDrawLib()->setColor(c1);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY1);
          getApp()->getDrawLib()->glVertexSP((cx+pFont->Chars[nChar].nOffsetX),(cy-pFont->Chars[nChar].nOffsetY));
	  getApp()->getDrawLib()->setColor(c2);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY1);        
          getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nWidth+pFont->Chars[nChar].nOffsetX,cy-pFont->Chars[nChar].nOffsetY);
	  getApp()->getDrawLib()->setColor(c3);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY2);
          getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nWidth+pFont->Chars[nChar].nOffsetX,cy+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY);
	  getApp()->getDrawLib()->setColor(c4);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY2);
          getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nOffsetX,cy+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY);
	  getApp()->getDrawLib()->endDraw();

          //glBegin(GL_POLYGON);
          //glColor4ub(GET_RED(c1),GET_GREEN(c1),GET_BLUE(c1),GET_ALPHA(c1));
          //glTexCoord2f(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY1);
          //getApp()->glVertexSP((cx-pFont->Chars[nChar].nOffsetY),(cy-pFont->Chars[nChar].nOffsetX));
          //glColor4ub(GET_RED(c2),GET_GREEN(c2),GET_BLUE(c2),GET_ALPHA(c2));
          //glTexCoord2f(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY1);        
          //getApp()->glVertexSP(cx-pFont->Chars[nChar].nOffsetY,cy-pFont->Chars[nChar].nOffsetX-pFont->Chars[nChar].nWidth);
          //glColor4ub(GET_RED(c3),GET_GREEN(c3),GET_BLUE(c3),GET_ALPHA(c3));
          //glTexCoord2f(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY2);
          //getApp()->glVertexSP(cx+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY,cy-pFont->Chars[nChar].nOffsetX-pFont->Chars[nChar].nWidth);
          //glColor4ub(GET_RED(c4),GET_GREEN(c4),GET_BLUE(c4),GET_ALPHA(c4));
          //glTexCoord2f(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY2);
          //getApp()->glVertexSP(cx+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY,(cy-pFont->Chars[nChar].nOffsetX));
          //glEnd();
          
          //glDisable(GL_TEXTURE_2D);
          //glBegin(GL_LINE_LOOP);
          //glColor4f(1,0,0,1);          
          //getApp()->glVertexSP((cx+pFont->Chars[nChar].nOffsetX),(cy-pFont->Chars[nChar].nOffsetY));
          //glColor4f(0,1,0,1);          
          //getApp()->glVertexSP(cx+pFont->Chars[nChar].nOffsetX,cy-pFont->Chars[nChar].nOffsetY-pFont->Chars[nChar].nWidth);
          //glColor4f(0,0,1,1);          
          //getApp()->glVertexSP(cx+pFont->Chars[nChar].nHeight+pFont->Chars[nChar].nOffsetX,cy-pFont->Chars[nChar].nOffsetY-pFont->Chars[nChar].nWidth);
          //glColor4f(0,0,0,1);          
          //getApp()->glVertexSP(cx+pFont->Chars[nChar].nHeight+pFont->Chars[nChar].nOffsetX,(cy-pFont->Chars[nChar].nOffsetY));
          //glEnd();
          //glEnable(GL_TEXTURE_2D);
          //
          cy += (pFont->Chars[nChar].nHeight*5)/4;
        }
        else {
	  getApp()->getDrawLib()->setTexture(pFont->pTexture,BLEND_MODE_A);
	  getApp()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	  getApp()->getDrawLib()->setColor(c1);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY1);
          getApp()->getDrawLib()->glVertexSP((cx+pFont->Chars[nChar].nOffsetX),(cy-pFont->Chars[nChar].nOffsetY));
	  getApp()->getDrawLib()->setColor(c2);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY1);        
          getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nWidth+pFont->Chars[nChar].nOffsetX,cy-pFont->Chars[nChar].nOffsetY);
	  getApp()->getDrawLib()->setColor(c3);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX2,pFont->Chars[nChar].fY2);
          getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nWidth+pFont->Chars[nChar].nOffsetX,cy+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY);
	  getApp()->getDrawLib()->setColor(c4);
          getApp()->getDrawLib()->glTexCoord(pFont->Chars[nChar].fX1,pFont->Chars[nChar].fY2);
          getApp()->getDrawLib()->glVertexSP(cx+pFont->Chars[nChar].nOffsetX,cy+pFont->Chars[nChar].nHeight-pFont->Chars[nChar].nOffsetY);
          getApp()->getDrawLib()->endDraw();
          
          cx += pFont->Chars[nChar].nIncX;
        }
      }
      else
        cx += pFont->Chars['-'].nIncX;
    }
  }
    
  void UITextDraw::getTextExt(UIFont *pFont,std::string Text,int *pnMinX,int *pnMinY,int *pnMaxX,int *pnMaxY) {
    int cx=0,cy=0;
    
    if(pnMinX) *pnMinX = 10000;
    if(pnMinY) *pnMinY = 10000;
    if(pnMaxX) *pnMaxX = -10000;
    if(pnMaxY) *pnMaxY = -10000;
    
    for(int i=0;i<Text.size();i++) {
      int nChar = Text[i];
      if(nChar == '\n') {
        cy += (pFont->Chars['|'].nHeight*3)/2;
        cx = 0;
      }
      else if(nChar == ' ') {
        cx += pFont->Chars['-'].nIncX;
        if(pnMaxX) *pnMaxX += pFont->Chars['-'].nIncX;
      }
      else if(pFont->Chars[nChar].bAvail) {
        int x1 = cx + pFont->Chars[nChar].nOffsetX;
        int y1 = cy - pFont->Chars[nChar].nOffsetY;
        int x2 = cx + pFont->Chars[nChar].nIncX;
        int y2 = y1 + pFont->Chars[nChar].nHeight;

        if(pnMinX && x1<*pnMinX) *pnMinX = x1;
        if(pnMinY && y1<*pnMinY) *pnMinY = y1;
        if(pnMaxX && x2>*pnMaxX) *pnMaxX = x2;
        if(pnMaxY && y2>*pnMaxY) *pnMaxY = y2;
        
        cx += pFont->Chars[nChar].nIncX;
      }
      else {
        cx += pFont->Chars['-'].nIncX;
        if(pnMaxX) *pnMaxX += pFont->Chars['-'].nIncX;
      }
    }  
  }
  
  int UITextDraw::getRefCount(void) {
    return m_nRefCount;
  }
  
  App *UITextDraw::getApp(void) {
    return m_pApp;
  }
    
  /* Static class data */
  std::vector<UIFont *> UITextDraw::m_Fonts;    
  int UITextDraw::m_nRefCount = 0;
  App *UITextDraw::m_pApp = NULL;

  /*===========================================================================
  Texture mangling
  ===========================================================================*/ 
  App *UITexture::getApp(void) {
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
  
  void UITexture::setApp(App *pApp) {
    m_pApp = pApp;
    
    Sprite *pSprite;

    pSprite = m_pApp->getTheme()->getSprite(SPRITE_TYPE_UI, "Misc");
    if(pSprite != NULL) {
      m_pUIElemTexture = pSprite->getTexture(false,true, FM_NEAREST);
    }

    pSprite = m_pApp->getTheme()->getSprite(SPRITE_TYPE_UI, "MiscDisabled");
    if(pSprite != NULL) {
      m_pUIElemTextureD = pSprite->getTexture(false,true, FM_NEAREST);
    }

    pSprite = m_pApp->getTheme()->getSprite(SPRITE_TYPE_UI, "MiscActive");
    if(pSprite != NULL) {
      m_pUIElemTextureA = pSprite->getTexture(false,true, FM_NEAREST);
    }
  }
  
  /* Static class data */  
  App *UITexture::m_pApp = NULL;
  Texture *UITexture::m_pUIElemTexture = NULL;
  Texture *UITexture::m_pUIElemTextureD = NULL;
  Texture *UITexture::m_pUIElemTextureA = NULL;

  /*===========================================================================
  Activation maps
  ===========================================================================*/
  int UIRoot::_UpdateActivationMap(UIWindow *pWindow,UIRootActCandidate *pMap,int nNum,int nMax) {
    if(nNum >= nMax) return nNum;
  
    /* Find all windows which accepts activations, and list them in a nice structure */
    if(!pWindow->isDisabled() && pWindow->offerActivation()) {
      pMap[nNum].x = pWindow->getAbsPosX() + pWindow->getPosition().nWidth / 2;
      pMap[nNum].y = pWindow->getAbsPosY() + pWindow->getPosition().nHeight / 2;
      pMap[nNum].pWindow = pWindow;
      nNum++;
    }
    
    /* Recurse */
    for(int i=0;i<pWindow->getChildren().size();i++) {
      if(!pWindow->isHidden() && !pWindow->isDisabled())
        nNum = _UpdateActivationMap(pWindow->getChildren()[i],pMap,nNum,nMax);
    }
    
    /* Return number of candidates */
    return nNum;
  }

  int UIRoot::_GetActiveIdx(UIRootActCandidate *pMap,int nNum) {
    for(int i=0;i<nNum;i++)
      if(pMap[i].pWindow->isActive())
        return i;
    return -1;
  }

  void UIRoot::_ActivateByVector(int dx,int dy) {
    /* Update the activation map */
    UIRootActCandidate Map[128];
    int nNum = _UpdateActivationMap(this,Map,0,128);
    
    if(nNum > 0) {
      /* Find the currently active window */
      int nActive = _GetActiveIdx(Map,nNum);
      
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
        int nBest=-1,nBestRating;
        for(int i=0;i<nNum;i++) {
          if(i!=nActive) {
            /* Calculate rating */
            int vx = Map[i].x - Map[nActive].x;
            int vy = Map[i].y - Map[nActive].y;
            int nDistSq = vx*vx + vy*vy;
            int nRating = 0;
            float xx1 = vx, yy1 = vy;
            float fDist = sqrt(xx1*xx1 + yy1*yy1);
            if(fDist == 0.0f) continue;
            xx1 /= fDist; yy1 /= fDist;
            float xx2 = dx, yy2 = dy;
           
            float fAngle;            
            fAngle = (acos(xx1*xx2 + yy1*yy2) / 3.14159f) * 180.0f;
            if((vx<0 && dx<0) || (vx>0 && dx>0) || (vy<0 && dy<0) || (vy>0 && dy>0)) {
              if(fAngle < 45.0f)
                nRating = 200-fAngle + 1000 - fDist;
            }
            
            /* Good? */
            if(nRating>0 && (nBest<0 || nRating > nBestRating)) {
              nBest = i;
              nBestRating = nRating;
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


}

