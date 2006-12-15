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

#ifndef __GUI_H__
#define __GUI_H__

#include "VCommon.h"
#include "VApp.h"

namespace vapp {

  class UIMsgBox;
  class UIButton;
  class UIRoot;

	/*===========================================================================
	Alignments
  ===========================================================================*/
  enum UIAlign {
    UI_ALIGN_TOP,
    UI_ALIGN_BOTTOM,
    UI_ALIGN_LEFT,
    UI_ALIGN_RIGHT,
    UI_ALIGN_CENTER
  };

	/*===========================================================================
	Elements
  ===========================================================================*/
  enum UIElem {
    UI_ELEM_LARGE_BUTTON_UP,
    UI_ELEM_LARGE_BUTTON_DOWN,
    UI_ELEM_SMALL_BUTTON_UP,
    UI_ELEM_SMALL_BUTTON_DOWN,
    UI_ELEM_CHECKBUTTON_UNCHECKED_UP,
    UI_ELEM_CHECKBUTTON_UNCHECKED_DOWN,
    UI_ELEM_CHECKBUTTON_CHECKED_UP,
    UI_ELEM_CHECKBUTTON_CHECKED_DOWN,
    UI_ELEM_RADIOBUTTON_UNCHECKED_UP,
    UI_ELEM_RADIOBUTTON_UNCHECKED_DOWN,
    UI_ELEM_RADIOBUTTON_CHECKED_UP,
    UI_ELEM_RADIOBUTTON_CHECKED_DOWN,
    UI_ELEM_SCROLLBUTTON_DOWN_UP,
    UI_ELEM_SCROLLBUTTON_DOWN_DOWN,
    UI_ELEM_SCROLLBUTTON_UP_UP,
    UI_ELEM_SCROLLBUTTON_UP_DOWN,
    UI_ELEM_SCROLLBUTTON_RIGHT_UP,
    UI_ELEM_SCROLLBUTTON_RIGHT_DOWN,
    UI_ELEM_SCROLLBUTTON_LEFT_UP,
    UI_ELEM_SCROLLBUTTON_LEFT_DOWN,
    UI_ELEM_FRAME_TL,
    UI_ELEM_FRAME_TM,
    UI_ELEM_FRAME_TR,
    UI_ELEM_FRAME_ML,
    UI_ELEM_FRAME_MM,
    UI_ELEM_FRAME_MR,
    UI_ELEM_FRAME_BL,
    UI_ELEM_FRAME_BM,
    UI_ELEM_FRAME_BR    
  };

	/*===========================================================================
	Rectangle
  ===========================================================================*/
  struct UIRect {
    UIRect() {
      nX = nY = nWidth = nHeight = 0;
    }
  
    int nX,nY,nWidth,nHeight;
  };

	/*===========================================================================
	Font character
  ===========================================================================*/
  struct UIFontChar {
    UIFontChar() {
      bAvail = false;
      nIncX = nIncY = nWidth = nHeight = nOffsetX = nOffsetY = 0;
      fX1 = fY1 = fX2 = fY2 = 0.0f;
    }
  
    bool bAvail;
    int nIncX,nIncY;
    int nWidth,nHeight;
    int nOffsetX,nOffsetY;
    float fX1,fY1,fX2,fY2;
  };

	/*===========================================================================
	Font
  ===========================================================================*/
  struct UIFont {
    UIFont() {
      pTexture = NULL;
    }
  
    std::string Name;
    UIFontChar Chars[256];
    Texture *pTexture;
  };

	/*===========================================================================
	Text style
  ===========================================================================*/
  struct UITextStyle {
    UITextStyle() {
      c0 = c1 = c2 = c3 = -1;
    }
  
    Color c0,c1,c2,c3;
  };

	/*===========================================================================
	UI texture handler static class (simple mechanism for not loading multiple
	copies of the same texture)
  ===========================================================================*/
  class UITexture {
    public:
      static App *getApp(void);
      static void setApp(App *pApp);
      static Texture *getMiscTexture(void);
      static Texture *getMiscDisabledTexture(void);
      static Texture *getMiscActiveTexture(void);
    
    private:
      static App *m_pApp;
      
      static Texture *m_pUIElemTexture;
      static Texture *m_pUIElemTextureD;
      static Texture *m_pUIElemTextureA;
  };

	/*===========================================================================
	UI text drawing static class
  ===========================================================================*/
  class UITextDraw {
    public:
      static void initTextDrawing(App *pApp);
      static void uninitTextDrawing(void);
      static UIFont *getFont(std::string Name);
      static void printRaw(UIFont *pFont,int x,int y,std::string Text,Color c);
      static void printRawGrad(UIFont *pFont,int x,int y,std::string Text,Color c1,Color c2,Color c3,Color c4,bool bRotated=false);
      static int getRefCount(void);
      static App *getApp(void);
      static void getTextExt(UIFont *pFont,std::string Text,int *pnMinX,int *pnMinY,int *pnMaxX,int *pnMaxY);
    
    private:
      static std::vector<UIFont *> m_Fonts;
      static App *m_pApp;
      static int m_nRefCount;
  };

	/*===========================================================================
	UI window
  ===========================================================================*/
  enum UIMsgBoxButton {
    UI_MSGBOX_NOTHING     = 0,    
    UI_MSGBOX_OK          = 1,
    UI_MSGBOX_CANCEL      = 2,
    UI_MSGBOX_YES         = 4,
    UI_MSGBOX_NO          = 8,
    UI_MSGBOX_YES_FOR_ALL = 16
  };
  
  class UIWindow {
    protected:
      void _InitWindow(void) {
        m_pParent=NULL;m_pApp=NULL;
        m_pCurFont=NULL;
        m_Pos.nX=m_Pos.nY=m_Pos.nWidth=m_Pos.nHeight=0;        
        m_bHide = false;
        m_bDisable = false;
        m_bActive = false;
        m_nGroup = 0;
        m_bActiveMsgBox = false;
        setPrimaryChild(NULL);
        setOpacity(100);
      }
      
    public:
      UIWindow() {}
      UIWindow(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {
        initW(pParent,x,y,Caption,nWidth,nHeight);
      }
      virtual ~UIWindow(void) {
        freeW();
      }
    
      /* Methods */
      virtual void paint(void) {}
      virtual void mouseWheelUp(int x,int y) {}
      virtual void mouseWheelDown(int x,int y) {}
      virtual void mouseLDown(int x,int y) {}
      virtual void mouseLDoubleClick(int  x,int y) {}
      virtual void mouseLUp(int x,int y) {}
      virtual void mouseRDown(int x,int y) {}
      virtual void mouseRUp(int x,int y) {}
      virtual void mouseHover(int x,int y) {}
      virtual bool keyDown(int nKey,int nChar) {return false;}
      virtual bool keyUp(int nKey) {return false;}
      virtual bool offerActivation(void) {return false;}
      virtual bool offerMouseEvent(void) {return true;}
      virtual std::string subContextHelp(int x,int y) {return "";}

      /* Painting */            
      void putText(int x,int y,std::string Text,bool bRotated=false);      
      void putImage(int x,int y,int nWidth,int nHeight,Texture *pImage);
      void putElem(int x,int y,int nWidth,int nHeight,UIElem Elem,bool bDisabled,bool bActive=false);
      void putRect(int x,int y,int nWidth,int nHeight,Color c);
      void setScissor(int x,int y,int nWidth,int nHeight);
      void getScissor(int *px,int *py,int *pnWidth,int *pnHeight);
      
      /* Utils */
      void getTextExt(std::string Text,int *pnMinX,int *pnMinY,int *pnMaxX,int *pnMaxY);
      void getTextSize(std::string Text,int *pnX,int *pnY);
      
      bool isMouseLDown(void);
      bool isMouseRDown(void);
      
      int getAbsPosX(void);
      int getAbsPosY(void);
      
      void makeActive(void);
      void enableChildren(bool bState);
      
      UIWindow *getChild(std::string Child);
      UIRoot *getRoot(void);
      
      /* Generic message boxing */
      UIMsgBox *msgBox(std::string Text,UIMsgBoxButton Buttons,bool bTextInput=false,bool bQuery=false);
    
      /* Data interface */
      UIWindow *getPrimaryChild(void) {return m_pPrimaryChild;}
      void setPrimaryChild(UIWindow *p) {m_pPrimaryChild = p;}
      UIFont *getFont(void) {return m_pCurFont;}
      void setFont(UIFont *pFont) {m_pCurFont=pFont;}
      UIWindow *getParent(void) {return m_pParent;}
      App *getApp(void) {return m_pApp;}
      void setApp(App *pApp) {m_pApp=pApp;}
      void setID(std::string ID) {m_ID=ID;}
      std::string getID(void) {return m_ID;}      
      std::vector<UIWindow *> &getChildren(void) {return m_Children;}
      UIRect &getPosition(void) {return m_Pos;}
      void setPosition(int x,int y,int nWidth,int nHeight) {m_Pos.nX=x; m_Pos.nY=y; m_Pos.nWidth=nWidth; m_Pos.nHeight=nHeight;}
      std::string getCaption(void) {return m_Caption;}
      void setCaption(std::string Caption) {m_Caption=Caption;}
      UITextStyle &getTextStyle(void) {return m_TextStyle;}
      void setTextSolidColor(Color c) {m_TextStyle.c0=m_TextStyle.c1=m_TextStyle.c2=m_TextStyle.c3=c;}
      void setTextGradientColors(Color a,Color b) {m_TextStyle.c0=m_TextStyle.c1=a; m_TextStyle.c2=m_TextStyle.c3=b;}
      void setOpacity(float fOpacity) {m_fOpacity = fOpacity;}
      float getLocalOpacity(void) {return m_fOpacity;}
      float getOpacity(void);
      bool isHidden(void) {return m_bHide;}
      bool isBranchHidden(void);
      bool isDisabled(void);
      void showWindow(bool b);      
      void enableWindow(bool b) {m_bDisable=!b; if(!b) m_bActive=false;}
      bool isActive(void) {return m_bActive;}
      void setActive(bool b) {m_bActive = b;}
      int getGroup(void) {return m_nGroup;}
      void setGroup(int n) {m_nGroup = n;}
      void setContextHelp(const std::string &s) {m_ContextHelp = s;}
      const std::string &getContextHelp(void) {return m_ContextHelp;}
      void clearMsgBoxActive(void) {m_bActiveMsgBox = false; /* pain */}
    
      bool isUglyMode() {return getApp()->isUglyMode();}

    protected:
      /* Protected interface */
      void addChildW(UIWindow *pWindow);
      void removeChildW(UIWindow *pWindow);
      bool haveChildW(UIWindow *pWindow);
      void initW(UIWindow *pParent,int x,int y,std::string Caption,int nWidth,int nHeight);
      void freeW(void);
    
    private:
      /* Data */
      std::string m_ContextHelp;                /* Context help */
      UIWindow *m_pPrimaryChild;                /* If !=null, this child is activated when
                                                   the window transists from "hidden" to "shown". 
                                                   useful for specifying "default" buttons in menus */
      UIWindow *m_pParent;                      /* Parent window */
      std::string m_ID;                         /* Non-unique id */
      std::vector<UIWindow *> m_Children;       /* Child windows */  
      App *m_pApp;                              /* Application */      
      UIRect m_Pos;                             /* Position */
      std::string m_Caption;                    /* Caption */
      UIFont *m_pCurFont;                       /* Current font */
      UITextStyle m_TextStyle;                  /* Current text style */
      float m_fOpacity;                         /* Opacity (0-100) */
      Texture *m_pUIElemTexture;                /* UI element texture */
      Texture *m_pUIElemTextureD;               /* UI element texture (disabled) */
      bool m_bHide;                             /* Hide window */
      bool m_bDisable;                          /* Disable window */
      bool m_bActive;                           /* Only one window should be active at a time */
      int m_nGroup;                             /* In window group */    
      bool m_bActiveMsgBox;                     /* Message box active */
  };  

	/*===========================================================================
	UI edit
  ===========================================================================*/
  class UIEdit : public UIWindow {
    public:
      UIEdit() {}
      UIEdit(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {
        initW(pParent,x,y,Caption,nWidth,nHeight);
        m_nCursorPos = 0;
	m_hideText = false;
      }      
    
      /* Methods */
      virtual void paint(void);
      virtual bool keyDown(int nKey,int nChar);
      
      virtual bool offerActivation(void) {return true;}
      void hideText(bool bHideText) {m_hideText=bHideText;} 

    private:
      /* Data */
      int m_nCursorPos;
      bool m_hideText;    
  };

	/*===========================================================================
	UI frame
  ===========================================================================*/
  enum UIFrameStyle {
    UI_FRAMESTYLE_MENU,                  /* "Iron-like" scaled menu */
    UI_FRAMESTYLE_TRANS,                 /* Generic transparent one */
    UI_FRAMESTYLE_LEFTTAG                /* Like above, but with left tag */
  };
  
  class UIFrame : public UIWindow {
    public:
      UIFrame();
      UIFrame(UIWindow *pParent,
	      int x=0,int y=0,
	      std::string Caption="",
	      int nWidth=0,int nHeight=0);    
    
      /* Methods */
      virtual void paint(void);
      virtual void mouseLDown(int x,int y);
      virtual void mouseHover(int x,int y);
      
      void makeMinimizable(int nMinX,int nMinY);
      void setMinimized(bool b);
      
      /* Data interface */
      UIFrameStyle getStyle(void) {return m_Style;}
      void setStyle(UIFrameStyle Style) {m_Style=Style;}
      
    private:
      /* Data */
      UIFrameStyle m_Style;
      
      bool m_bHover;
      bool m_bMinimizable;
      bool m_bMinimized;
      int m_nMinimizedX,m_nMinimizedY; /* Minimized position */
      int m_nMaximizedX,m_nMaximizedY; /* Maximized position */
      float m_fMinMaxTime;
      Texture *m_pMenuTL,*m_pMenuTR,*m_pMenuBL,*m_pMenuBR;
  };

	/*===========================================================================
	UI highscore list
  ===========================================================================*/
  class UIBestTimes : public UIFrame {
    public:
      UIBestTimes() {}
      UIBestTimes(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {
        initW(pParent,x,y,Caption,nWidth,nHeight);

        setStyle(UI_FRAMESTYLE_TRANS);
      }

      /* Virtual methods */
      virtual void paint(void);      
      
      /* Setup */
      void setup(std::string Header,int n1,int n2) {m_Header=Header; m_nHighlight1=n1; m_nHighlight2=n2;}
      void addRow1(std::string Col1,std::string Col2) {m_Col1.push_back(Col1); m_Col2.push_back(Col2);}
      void addRow2(std::string Col1,std::string Col2) {m_Col3.push_back(Col1); m_Col4.push_back(Col2);}
      void setHFont(UIFont *pFont) {m_pHFont=pFont;}
      void clear(void);
      
    private:
      /* Data */
      std::string m_Header;
      std::vector<std::string> m_Col1;
      std::vector<std::string> m_Col2;
      std::vector<std::string> m_Col3;
      std::vector<std::string> m_Col4;
      int m_nHighlight1,m_nHighlight2;
      UIFont *m_pHFont;
  };

	/*===========================================================================
	UI message box
  ===========================================================================*/
  class UIMsgBox : public UIFrame {
    public:
      UIMsgBox() {m_bTextInput = false;m_nNumButtons=0;}
      UIMsgBox(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {
        initW(pParent,x,y,Caption,nWidth,nHeight);

        setStyle(UI_FRAMESTYLE_TRANS);
        m_pTextInputFont = NULL;
        m_bTextInput = false;
        m_nNumButtons = 0;
      }      
      virtual ~UIMsgBox() {_ReEnableSiblings();}
    
      /* Virtual methods */
      virtual void paint(void);      
      virtual bool keyDown(int nKey,int nChar);
      virtual bool offerActivation(void) {if(m_bTextInput) return true; return false;}
    
      /* Methods */
      bool setClicked(std::string Text);
      UIMsgBoxButton getClicked(void);
      void enableTextInput(void) {m_bTextInput=true;}
      std::string getTextInput(void) {return m_TextInput;}
      void setTextInput(std::string s) {m_TextInput=s;}
      void setTextInputFont(UIFont *pFont) {m_pTextInputFont = pFont;}
      
      /* Data interface */
      void addButton(UIButton *p) {m_pButtons[m_nNumButtons++] = p;}
      std::vector<bool> &getSiblingStates(void) {return m_SiblingStates;}
      
    private:
      /* Data */
      std::vector<bool> m_SiblingStates;
      UIMsgBoxButton m_Clicked;
      UIButton *m_pButtons[10];
      int m_nNumButtons;
      
      bool m_bTextInput;
      std::string m_TextInput;
      UIFont *m_pTextInputFont;
      
      /* Helpers */
      void _ReEnableSiblings(void);
  };

	/*===========================================================================
	Special message box for input key querying 
  ===========================================================================*/
  class UIQueryKeyBox : public UIMsgBox {
    public:
      UIQueryKeyBox() {}
      UIQueryKeyBox(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {
        initW(pParent,x,y,Caption,nWidth,nHeight);

        setStyle(UI_FRAMESTYLE_TRANS);
        setTextInputFont(NULL);
      }      

      /* Virtual methods */
      virtual bool keyDown(int nKey,int nChar);
  };

	/*===========================================================================
	UI static
  ===========================================================================*/
  class UIStatic : public UIWindow {
    public:
      UIStatic() {}
      UIStatic(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {
        initW(pParent,x,y,Caption,nWidth,nHeight);
        
        m_VAlign = UI_ALIGN_CENTER;
        m_HAlign = UI_ALIGN_CENTER;
        
        m_bBackgroundShade = false;
        
        m_pDarkBlobTexture = NULL;
	Sprite *pSprite;
	pSprite = getApp()->m_theme.getSprite(SPRITE_TYPE_UI, "DarkBlob");
	if(pSprite != NULL) {
	  m_pDarkBlobTexture = pSprite->getTexture(false,true, FM_NEAREST);
	}
        
        m_pCustomBackgroundTexture = NULL;
      }      

      /* Methods */
      virtual void paint(void);
      virtual bool offerMouseEvent(void) {return false;}
      
      /* Data interface */
      void setVAlign(UIAlign Align) {m_VAlign=Align;}
      void setHAlign(UIAlign Align) {m_HAlign=Align;}
      UIAlign getVAlign(void) {return m_VAlign;}
      UIAlign getHAlign(void) {return m_HAlign;}
      void setBackgroundShade(bool b) {m_bBackgroundShade=b;}
      void setBackground(Texture *p) {m_pCustomBackgroundTexture = p;}

    private:
      /* Data */
      UIAlign m_VAlign,m_HAlign;
      bool m_bBackgroundShade;

      Texture *m_pDarkBlobTexture;
      Texture *m_pCustomBackgroundTexture;
  };
  
	/*===========================================================================
	UI button
  ===========================================================================*/
  enum UIButtonState {
    UI_BUTTON_STATE_UNPRESSED,
    UI_BUTTON_STATE_PRESSED
  };
  
  enum UIButtonType {
    UI_BUTTON_TYPE_LARGE,
    UI_BUTTON_TYPE_SMALL,
    UI_BUTTON_TYPE_CHECK,
    UI_BUTTON_TYPE_RADIO
  };
  
  class UIButton : public UIWindow {
    public:
      UIButton() {}
      UIButton(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {      
        initW(pParent,x,y,Caption,nWidth,nHeight);
        
        m_State = UI_BUTTON_STATE_UNPRESSED;
        m_Type = UI_BUTTON_TYPE_LARGE;
        m_bClicked = false;
        m_bHover = false;
        m_bChecked = false;
      }      

      /* Methods */
      virtual void paint(void);
      virtual void mouseLDown(int x,int y);
      virtual void mouseLUp(int x,int y);
      virtual void mouseRDown(int x,int y);
      virtual void mouseRUp(int x,int y);
      virtual void mouseHover(int x,int y);      
      virtual bool offerActivation(void);
      virtual bool keyDown(int nKey,int nChar);
      
      /* Data interface */
      UIButtonState getState(void) {return m_State;}
      void setState(UIButtonState State) {m_State = State;}
      bool isClicked(void) {return m_bClicked;}
      UIButtonType getType(void) {return m_Type;}
      void setType(UIButtonType Type) {m_Type = Type;}
      void setClicked(bool b) {m_bClicked=b;}
      void setChecked(bool b) {
        if(m_Type == UI_BUTTON_TYPE_RADIO) _UncheckGroup(getGroup());
        m_bChecked=b;
      }
      bool getChecked(void) {return m_bChecked;}
      
    private:
      /* Data */
      UIButtonState m_State;
      UIButtonType m_Type;            
      bool m_bHover,m_bClicked,m_bChecked;
      
      /* Helper methods */
      void _UncheckGroup(int nGroup);
  };

	/*===========================================================================
	UI list
  ===========================================================================*/
  struct UIListEntry {
    std::vector<std::string> Text;
    void *pvUser;
  };
  
  class UIList : public UIWindow {
    public:
      UIList() {}
      UIList(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {      
        initW(pParent,x,y,Caption,nWidth,nHeight);        
        m_nScroll = 0;
        m_nSelected = 0;
        m_pEnterButton = NULL;
        m_bSort = false;
	      m_fsort = NULL;
	      m_bNumeroted = false;
        m_bItemActivated = false;
        
        m_bScrollDownPressed = m_bScrollUpPressed = false;
        
        m_bScrollDownHover = m_bScrollUpHover = false;
        
	      m_bClicked = false;

	      m_bChanged = false;

	      m_bScrolling = false;

	      m_lastRefreshTime = getApp()->getRealTime();

        unhideAllColumns();
      }      
      virtual ~UIList() {_FreeUIList();}

      /* Methods */
      virtual void paint(void);
      virtual void mouseLDown(int x,int y);
      virtual void mouseLDoubleClick(int  x,int y);
      virtual void mouseLUp(int x,int y);
      virtual void mouseRDown(int x,int y);
      virtual void mouseRUp(int x,int y);
      virtual void mouseWheelUp(int x,int y);
      virtual void mouseWheelDown(int x,int y);
      virtual void mouseHover(int x,int y);      
      virtual bool offerActivation(void);
      virtual bool keyDown(int nKey,int nChar);

      virtual std::string subContextHelp(int x,int y);
      
      UIListEntry *addEntry(std::string Text,void *pvUser=NULL);
      void clear(void);
      
      /* Data interface */
      std::vector<UIListEntry *> &getEntries(void) {return m_Entries;}      
      std::vector<std::string> &getColumns(void) {return m_Columns;}
      int getSelected(void) {return m_nSelected;}
      void setSelected(int n);
      void addColumn(std::string Title,int nWidth,const std::string &Help = "") {m_Columns.push_back(Title); m_ColumnWidths.push_back(nWidth); m_ColumnHelpStrings.push_back(Help);}
      void setEnterButton(UIButton *pButton) {m_pEnterButton = pButton;}
      bool isItemActivated(void) {return m_bItemActivated;}
      void setHideColumn(int n) {m_nColumnHideFlags |= (1<<n);}
      void unhideAllColumns(void) {m_nColumnHideFlags=0;}
      void setSort(bool bSort, int(*f)(void *pvUser1, void *pvUser2) = NULL) {m_bSort=bSort; m_fsort = f;}
      void setNumeroted(bool bNumeroted) {m_bNumeroted = bNumeroted;}

      bool isClicked(void) {return m_bClicked;}
      void setClicked(bool b) {m_bClicked=b;}

      bool isChanged(void) {return m_bChanged;}
      void setChanged(bool b) {m_bChanged=b;}

    private:
      /* Data */
      bool m_bChanged;
      float m_lastRefreshTime;
      bool m_bSort;
      bool m_bNumeroted;
      int(*m_fsort)(void *pvUser1, void *pvUser2);
      std::vector<UIListEntry *> m_Entries;
      std::vector<std::string> m_Columns;
      std::vector<std::string> m_ColumnHelpStrings;
      std::vector<int> m_ColumnWidths;
      unsigned int m_nColumnHideFlags;
      int m_nSelected;
      bool m_bItemActivated;
      UIButton *m_pEnterButton; /* if not null this is the "default" button of the list, i.e. the one 
                                   that gets pressed if the list gets an enter */
      int m_nScroll;
      bool m_bScrollUpPressed,m_bScrollDownPressed;
      
      bool m_bScrollUpHover,m_bScrollDownHover;
      
      bool m_bClicked;
      bool  m_bScrolling;

      /* Helpers */
      void _FreeUIList(void);
      void _NewlySelectedItem(void);
      void _Scroll(int nPixels);
      void _refreshByTime();
      void _mouseDownManageScrollBar(int x, int y);
  };

	/*===========================================================================
	UI tab view
  ===========================================================================*/  
  class UITabView : public UIWindow {
    public:
      UITabView() {}
      UITabView(UIWindow *pParent,int x=0,int y=0,std::string Caption="",int nWidth=0,int nHeight=0) {      
        initW(pParent,x,y,Caption,nWidth,nHeight);        
        
        m_nSelected = 0;
	m_bChanged = false;
      }      

      /* Methods */
      virtual void paint(void);
      virtual void mouseLDown(int x,int y);
      virtual void mouseLUp(int x,int y);
      virtual void mouseRDown(int x,int y);
      virtual void mouseRUp(int x,int y);
      virtual void mouseHover(int x,int y);      
      virtual std::string subContextHelp(int x,int y);
      
      void setTabContextHelp(int nTab,const std::string &s);

      /* Data interface */      
      UIWindow *getSelected(void) {
        if(!getChildren().empty() && m_nSelected >= 0 && m_nSelected < getChildren().size()) 
          return getChildren()[m_nSelected];
        return NULL;
      }
      void setSelected(int n) {
        m_nSelected = n;
      }

      bool isChanged(void) {return m_bChanged;}
      void setChanged(bool b) {m_bChanged=b;}

    private:
      /* Data */
      bool m_bChanged;
      int m_nSelected;
      std::vector<std::string> m_TabContextHelp;
  };

	/*===========================================================================
	UI root
  ===========================================================================*/
  enum UIRootMouseEvent {
    UI_ROOT_MOUSE_LBUTTON_DOWN,
    UI_ROOT_MOUSE_LBUTTON_UP,
    UI_ROOT_MOUSE_RBUTTON_DOWN,
    UI_ROOT_MOUSE_RBUTTON_UP,
    UI_ROOT_MOUSE_HOVER,
    UI_ROOT_MOUSE_WHEEL_DOWN,
    UI_ROOT_MOUSE_WHEEL_UP,
    UI_ROOT_MOUSE_DOUBLE_CLICK
  };
  
  enum UIRootKeyEvent {
    UI_ROOT_KEY_DOWN,
    UI_ROOT_KEY_UP
  };
  
  struct UIRootActCandidate {
    UIWindow *pWindow;
    int x,y;
  };
  
  class UIRoot : public UIWindow {
    public:
      UIRoot() {
        _InitWindow();        
      }
    
      /* Methods */
      virtual void paint(void);            
      virtual void mouseLDoubleClick(int  x,int y);
      virtual void mouseLDown(int x,int y);
      virtual void mouseLUp(int x,int y);
      virtual void mouseRDown(int x,int y);
      virtual void mouseRUp(int x,int y);
      virtual void mouseHover(int x,int y);
      virtual void mouseWheelUp(int x,int y);
      virtual void mouseWheelDown(int x,int y);
      virtual bool keyDown(int nKey,int nChar);      
      virtual bool keyUp(int nKey);  
      
      void deactivate(UIWindow *pWindow);
      
      void enableContextMenuDrawing(bool b) {m_bShowContextMenu=b;}
      void clearContext(void) {m_CurrentContextHelp = "";}
      
      void activateUp(void);
      void activateDown(void);
      void activateLeft(void);
      void activateRight(void);
            
    private:
      /* Data */
      bool m_bShowContextMenu;
      std::string m_CurrentContextHelp;
            
      /* Helpers */      
      bool _RootKeyEvent(UIWindow *pWindow,UIRootKeyEvent Event,int nKey,int nChar);
      bool _RootMouseEvent(UIWindow *pWindow,UIRootMouseEvent Event,int x,int y);
      void _RootPaint(int x,int y,UIWindow *pWindow,UIRect *pRect);
      void _ClipRect(UIRect *pRect,UIRect *pClipWith);
      int _UpdateActivationMap(UIWindow *pWindow,UIRootActCandidate *pMap,int nNum,int nMax);
      void _ActivateByVector(int dx,int dy);
      int _GetActiveIdx(UIRootActCandidate *pMap,int nNum);
  };

}

#endif
