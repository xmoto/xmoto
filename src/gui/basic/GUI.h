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

#ifndef __GUI_H__
#define __GUI_H__

class UIMsgBox;
class UIButton;
class UIRoot;
class DrawLib;
class FontManager;
class Sprite;
class GameApp;
class RenderSurface;

#include "common/TextEdit.h"
#include "common/Theme.h"
#include "helpers/VMath.h"
#include "include/xm_SDL.h"
#include "xmoto/input/XMKey.h"

#define UGLY_MODE_WINDOW_BG MAKE_COLOR(35, 35, 35, 255)

static const int IBeamWidth = 1;
static const int BlockCursorWidth = 6;

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
  UIRect() { nX = nY = nWidth = nHeight = 0; }

  int nX, nY, nWidth, nHeight;
};

/*===========================================================================
  Text style
  ===========================================================================*/
struct UITextStyle {
  UITextStyle() { c0 = c1 = c2 = c3 = (Color)-1; }

  Color c0, c1, c2, c3;
};

/*===========================================================================
  UI texture handler static class (simple mechanism for not loading multiple
  copies of the same texture)
  ===========================================================================*/
class UITexture {
public:
  static GameApp *getApp(void);
  static void setApp(GameApp *pApp);
  static Texture *getMiscTexture(void);
  static Texture *getMiscDisabledTexture(void);
  static Texture *getMiscActiveTexture(void);

private:
  static GameApp *m_pApp;

  static Texture *m_pUIElemTexture;
  static Texture *m_pUIElemTextureD;
  static Texture *m_pUIElemTextureA;
};

/*===========================================================================
  UI window
  ===========================================================================*/
enum UIMsgBoxButton {
  UI_MSGBOX_NOTHING = 0,
  UI_MSGBOX_OK = (1 << 0),
  UI_MSGBOX_CANCEL = (1 << 1),
  UI_MSGBOX_OPTIONS = (1 << 2),
  UI_MSGBOX_QUIT = (1 << 3),
  UI_MSGBOX_YES = (1 << 4),
  UI_MSGBOX_NO = (1 << 5),
  UI_MSGBOX_YES_FOR_ALL = (1 << 6),
  UI_MSGBOX_CUSTOM1 = (1 << 7),
  UI_MSGBOX_CUSTOM2 = (1 << 8)
};

class UIWindow {
protected:
  void _InitWindow(void) {
    m_pParent = NULL;
    m_Pos.nX = m_Pos.nY = m_Pos.nWidth = m_Pos.nHeight = 0;
    m_bHide = false;
    m_bDisable = false;
    m_bActive = false;
    m_nGroup = 0;
    m_bActiveMsgBox = false;
    setPrimaryChild(NULL);
    setOpacity(100);
    m_canGetFocus = true;
  }

public:
  UIWindow() {
    m_curFont = NULL;
    m_canGetFocus = true;
  }
  UIWindow(UIWindow *pParent,
           int x = 0,
           int y = 0,
           std::string Caption = "",
           int nWidth = 0,
           int nHeight = 0) {
    initW(pParent, x, y, Caption, nWidth, nHeight);
  }
  virtual ~UIWindow(void) { freeW(); }

  static void setDrawLib(DrawLib *i_drawLib);

  /* Methods */
  virtual void paint(void) {}
  virtual void mouseWheelUp(int x, int y) {}
  virtual void mouseWheelDown(int x, int y) {}
  virtual void mouseLDown(int x, int y) {}
  virtual void mouseLDoubleClick(int x, int y) {}
  virtual void mouseLUp(int x, int y) {}
  virtual void mouseRDown(int x, int y) {}
  virtual void mouseRUp(int x, int y) {}
  virtual void mouseHover(int x, int y) {}
  virtual void mouseOut(int x, int y) {}
  virtual bool keyDown(int nKey,
                       SDL_Keymod mod,
                       const std::string &i_utf8Char) {
    return false;
  }
  virtual bool keyUp(int nKey, SDL_Keymod mod, const std::string &i_utf8Char) {
    return false;
  }
  virtual bool textInput(int nKey,
                         SDL_Keymod mod,
                         const std::string &i_utf8Char) {
    return false;
  }
  virtual bool joystickAxisMotion(JoyAxisEvent event) { return false; }
  virtual bool joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
    return false;
  }
  virtual bool offerActivation(void) { return false; }
  virtual bool offerMouseEvent(void) { return true; }
  virtual std::string subContextHelp(int x, int y) { return ""; }

  /* Painting */
  void putText(int x,
               int y,
               std::string Text,
               float i_xper = 0.0,
               float i_yper = 0.0,
               float i_perCentered = -1.0);
  void putTextS(int x,
                int y,
                std::string Text,
                int &o_width,
                int &o_height,
                float i_xper = 0.0,
                float i_yper = 0.0,
                float i_perCentered = -1.0);
  void putImage(int x, int y, int nWidth, int nHeight, Texture *pImage);
  void putElem(int x,
               int y,
               int nWidth,
               int nHeight,
               UIElem Elem,
               bool bDisabled,
               bool bActive = false);
  void putRect(int x, int y, int nWidth, int nHeight, Color c);
  void putPolygon(const std::vector<Vector2f> &points, Color c);
  void putPolygon(const std::vector<Vector2f> &points, Vector2f shift, Color c);
  void setScissor(int x, int y, int nWidth, int nHeight);
  void getScissor(int *px, int *py, int *pnWidth, int *pnHeight);

  bool isMouseLDown(void);
  bool isMouseRDown(void);

  int getAbsPosX(void);
  int getAbsPosY(void);

  void makeActive(void);
  void enableChildren(bool bState);

  UIWindow *getChild(std::string Child);
  UIRoot *getRoot(void);
  RenderSurface *getScreen();

  /* Generic message boxing */
  UIMsgBox *msgBox(std::string Text,
                   UIMsgBoxButton Buttons,
                   const std::string &i_help,
                   const std::string &i_custom1 = "",
                   const std::string &i_custom2 = "",
                   bool bTextInput = false,
                   bool bQuery = false,
                   bool i_verticallyLarge = false);
  UIMsgBox *msgBox(std::string Text,
                   std::vector<std::string> &wordcompletionlist,
                   UIMsgBoxButton Buttons,
                   const std::string &i_help,
                   const std::string &i_custom1 = "",
                   const std::string &i_custom2 = "",
                   bool bTextInput = false,
                   bool bQuery = false,
                   bool i_verticallyLarge = false);

  /* Data interface */
  UIWindow *getPrimaryChild(void) { return m_pPrimaryChild; }
  void setPrimaryChild(UIWindow *p) {
    m_pPrimaryChild = p;
    if (m_pPrimaryChild != NULL) {
      m_pPrimaryChild->setActive(true);
    }
  }
  FontManager *getFont() { return m_curFont; }
  void setFont(FontManager *pFont) { m_curFont = pFont; }
  UIWindow *getParent(void) { return m_pParent; }
  virtual GameApp *getApp(void) { return getParent()->getApp(); }
  void setID(std::string ID) { m_ID = ID; }
  std::string getID(void) { return m_ID; }
  std::vector<UIWindow *> &getChildren(void) { return m_Children; }
  UIRect &getPosition(void) { return m_Pos; }
  void setPosition(int x, int y, int nWidth, int nHeight) {
    m_Pos.nX = x;
    m_Pos.nY = y;
    m_Pos.nWidth = nWidth;
    m_Pos.nHeight = nHeight;
  }
  std::string getCaption(void) { return m_Caption; }
  virtual void setCaption(const std::string &Caption) { m_Caption = Caption; }
  UITextStyle &getTextStyle(void) { return m_TextStyle; }
  void setTextSolidColor(Color c) {
    m_TextStyle.c0 = m_TextStyle.c1 = m_TextStyle.c2 = m_TextStyle.c3 = c;
  }
  void setTextGradientColors(Color a, Color b) {
    m_TextStyle.c0 = m_TextStyle.c1 = a;
    m_TextStyle.c2 = m_TextStyle.c3 = b;
  }
  void setOpacity(float fOpacity) { m_fOpacity = fOpacity; }
  float getLocalOpacity(void) { return m_fOpacity; }
  float getOpacity(void);
  bool isHidden(void) { return m_bHide; }
  bool isBranchHidden(void);
  bool isDisabled(void); // recursive to parents
  bool isVisible(); // recursive to parents
  bool canGetFocus(void) { return m_canGetFocus && isHidden() == false; }
  void setCanGetFocus(bool b) { m_canGetFocus = b; }
  void showWindow(bool b);
  void enableWindow(bool b) { m_bDisable = !b; }
  bool isActive(void);
  void setActive(bool b) { m_bActive = b; }
  int getGroup(void) { return m_nGroup; }
  void setGroup(int n) { m_nGroup = n; }
  void setContextHelp(const std::string &s) { m_ContextHelp = s; }
  const std::string &getContextHelp(void) { return m_ContextHelp; }
  void clearMsgBoxActive(void) { m_bActiveMsgBox = false; /* pain */ }

  bool isUglyMode();

protected:
  /* Protected interface */
  void addChildW(UIWindow *pWindow);
  void removeChildW(UIWindow *pWindow);
  bool haveChildW(UIWindow *pWindow);
  void initW(UIWindow *pParent,
             int x,
             int y,
             std::string Caption,
             int nWidth,
             int nHeight);
  void freeW(void);

  static DrawLib *m_drawLib;

  // screen to draw ; the same as the parent, but copy to not get in cascade
  RenderSurface *m_screen;

private:
  /* Data */
  std::string m_ContextHelp; /* Context help */
  UIWindow
    *m_pPrimaryChild; /* If !=null, this child is activated when
                         the window transists from "hidden" to "shown".
                         useful for specifying "default" buttons in menus */
  UIWindow *m_pParent; /* Parent window */

  std::string m_ID; /* Non-unique id */
  std::vector<UIWindow *> m_Children; /* Child windows */
  UIRect m_Pos; /* Position */
  std::string m_Caption; /* Caption */
  FontManager *m_curFont;
  UITextStyle m_TextStyle; /* Current text style */
  float m_fOpacity; /* Opacity (0-100) */
  Texture *m_pUIElemTexture; /* UI element texture */
  Texture *m_pUIElemTextureD; /* UI element texture (disabled) */
  bool m_bHide; /* Hide window */
  bool m_bDisable; /* Disable window */
  bool m_bActive; /* Only one window should be active at a time */
  int m_nGroup; /* In window group */
  bool m_bActiveMsgBox; /* Message box active */
  bool m_canGetFocus; /* UI element can be focused with keyboard */
};

/*===========================================================================
  UI edit
  ===========================================================================*/
class UIEdit : public UIWindow {
public:
  UIEdit() {}
  UIEdit(UIWindow *pParent,
         int x = 0,
         int y = 0,
         std::string Caption = "",
         int nWidth = 0,
         int nHeight = 0) {
    initW(pParent, x, y, Caption, nWidth, nHeight);
    hideText(false);
    m_hasChanged = false;
  }

  /* Methods */
  virtual void paint(void);
  virtual bool keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char);
  virtual bool textInput(int nKey,
                         SDL_Keymod mod,
                         const std::string &i_utf8Char);
  virtual bool joystickAxisMotion(JoyAxisEvent event);
  virtual bool joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton);

  virtual bool offerActivation(void) { return true; }
  void hideText(bool bHideText) {
    m_hideText = bHideText;
    m_textEdit.setHidden(bHideText);
  }

  void setCaption(const std::string &Caption) {
    m_textEdit.setText(Caption);
    m_textEdit.jumpToEnd();
  }
  void updateCaption();

  void setHasChanged(bool b_value);
  bool hasChanged();

private:
  /* Data */
  TextEdit m_textEdit;
  bool m_hideText;
  bool m_hasChanged;
};

/*===========================================================================
  UI frame
  ===========================================================================*/
enum UIFrameStyle {
  UI_FRAMESTYLE_MENU, /* "Iron-like" scaled menu */
  UI_FRAMESTYLE_TRANS, /* Generic transparent one */
  UI_FRAMESTYLE_LEFTTAG /* Like above, but with left tag */
};

class UIFrame : public UIWindow {
public:
  UIFrame();
  UIFrame(UIWindow *pParent,
          int x = 0,
          int y = 0,
          std::string Caption = "",
          int nWidth = 0,
          int nHeight = 0);

  /* Methods */
  virtual void paint(void);
  virtual void mouseLDown(int x, int y);
  virtual void mouseHover(int x, int y);
  virtual void mouseOut(int x, int y);

  void makeMinimizable(int nMinX, int nMinY);
  void setMinimized(bool b);
  void toggle();

  bool isMinimized() const { return m_bMinimized; }

  /* Data interface */
  UIFrameStyle getStyle(void) { return m_Style; }
  void setStyle(UIFrameStyle Style) { m_Style = Style; }

private:
  /* Data */
  UIFrameStyle m_Style;

  bool m_bHover;
  bool m_bMinimizable;
  bool m_bMinimized;
  int m_nMinimizedX, m_nMinimizedY; /* Minimized position */
  int m_nMaximizedX, m_nMaximizedY; /* Maximized position */
  float m_fMinMaxTime;
  Texture *m_pMenuTL, *m_pMenuTR, *m_pMenuBL, *m_pMenuBR;
};

/*===========================================================================
  UI highscore list
  ===========================================================================*/
class UIBestTimes : public UIFrame {
public:
  UIBestTimes() {}
  UIBestTimes(UIWindow *pParent,
              int x = 0,
              int y = 0,
              std::string Caption = "",
              int nWidth = 0,
              int nHeight = 0) {
    initW(pParent, x, y, Caption, nWidth, nHeight);

    setStyle(UI_FRAMESTYLE_TRANS);
  }

  /* Virtual methods */
  virtual void paint(void);

  /* Setup */
  void setup(std::string Header, int n1, int n2) {
    m_Header = Header;
    m_nHighlight1 = n1;
    m_nHighlight2 = n2;
  }
  void addRow1(std::string Col1, std::string Col2) {
    m_Col1.push_back(Col1);
    m_Col2.push_back(Col2);
  }
  void addRow2(std::string Col1, std::string Col2) {
    m_Col3.push_back(Col1);
    m_Col4.push_back(Col2);
  }
  void setHFont(FontManager *pFont) { m_hFont = pFont; }
  void clear(void);

private:
  /* Data */
  std::string m_Header;
  std::vector<std::string> m_Col1;
  std::vector<std::string> m_Col2;
  std::vector<std::string> m_Col3;
  std::vector<std::string> m_Col4;
  int m_nHighlight1, m_nHighlight2;
  FontManager *m_hFont;
};

/*===========================================================================
  UI message box
  ===========================================================================*/
class UIMsgBox : public UIFrame {
public:
  UIMsgBox() {
    m_bTextInput = false;
    m_nNumButtons = 0;
  }
  UIMsgBox(UIWindow *pParent,
           int x = 0,
           int y = 0,
           std::string Caption = "",
           int nWidth = 0,
           int nHeight = 0);
  UIMsgBox(UIWindow *pParent,
           std::vector<std::string> &wordcompletionlist,
           int x = 0,
           int y = 0,
           std::string Caption = "",
           int nWidth = 0,
           int nHeight = 0);
  virtual ~UIMsgBox() { _ReEnableSiblings(); }

  /* Virtual methods */
  virtual void paint(void);
  virtual bool keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char);
  virtual bool textInput(int nKey,
                         SDL_Keymod mod,
                         const std::string &i_utf8Char);
  virtual bool offerActivation(void) { return m_bTextInput; }

  /* Methods */
  bool setClicked(std::string Text);
  UIMsgBoxButton getClicked(void);
  void enableTextInput(void) { m_bTextInput = true; }
  std::string getTextInput(void) { return m_displayText; }
  void setTextInput(const std::string &s) {
    m_displayText = s;
    m_textEdit.setText(s);
    m_textEdit.jumpToEnd();
  }
  void setTextInputFont(FontManager *pFont) { m_textInputFont = pFont; }
  void addCompletionWord(std::string &word);
  void addCompletionWord(std::vector<std::string> &list);

  std::string getCustom1() const;
  void setCustom1(const std::string &i_str);

  std::string getCustom2() const;
  void setCustom2(const std::string &i_str);

  /* Data interface */
  void addButton(UIButton *p) { m_pButtons[m_nNumButtons++] = p; }
  std::vector<bool> &getSiblingStates(void) { return m_SiblingStates; }

  void makeActiveButton(UIMsgBoxButton i_button);

private:
  void initMsgBox(UIWindow *pParent,
                  int x,
                  int y,
                  std::string Caption,
                  int nWidth,
                  int nHeight);

  /* Data */
  std::vector<bool> m_SiblingStates;
  UIMsgBoxButton m_Clicked;
  UIButton *m_pButtons[10];
  unsigned int m_nNumButtons;

  std::string m_custom1, m_custom2;

  bool m_bTextInput;
  TextEdit m_textEdit;
  std::string m_displayText;

  FontManager *m_textInputFont;
  std::vector<std::string> m_completionWords;

  /* Helpers */
  void _ReEnableSiblings(void);
  std::vector<std::string> findMatches();
  void showMatch(bool reverse);
};

/*===========================================================================
  Special message box for input key querying
  ===========================================================================*/
class UIQueryKeyBox : public UIMsgBox {
public:
  UIQueryKeyBox() {}
  UIQueryKeyBox(UIWindow *pParent,
                int x = 0,
                int y = 0,
                std::string Caption = "",
                int nWidth = 0,
                int nHeight = 0) {
    initW(pParent, x, y, Caption, nWidth, nHeight);

    setStyle(UI_FRAMESTYLE_TRANS);
    setTextInputFont(NULL);
  }

  /* Virtual methods */
  virtual bool keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char);
};

/*===========================================================================
  UI static
  ===========================================================================*/
class UIStatic : public UIWindow {
public:
  UIStatic() {}
  UIStatic(UIWindow *pParent,
           int x = 0,
           int y = 0,
           std::string Caption = "",
           int nWidth = 0,
           int nHeight = 0);

  /* Methods */
  virtual void paint(void);
  virtual bool offerMouseEvent(void) { return m_allowContextHelp; }

  /* Data interface */
  void setVAlign(UIAlign Align) { m_VAlign = Align; }
  void setHAlign(UIAlign Align) { m_HAlign = Align; }
  UIAlign getVAlign(void) { return m_VAlign; }
  UIAlign getHAlign(void) { return m_HAlign; }
  void setBackgroundShade(bool b) { m_bBackgroundShade = b; }
  void setBackground(Texture *p) { m_pCustomBackgroundTexture = p; }
  void setAllowContextHelp(bool i_value);
  void setNormalColor(Color c);

private:
  /* Data */
  UIAlign m_VAlign, m_HAlign;
  bool m_bBackgroundShade;

  Texture *m_pDarkBlobTexture;
  Texture *m_pCustomBackgroundTexture;

  bool m_allowContextHelp;
  Color m_normalColor;
};

class UIProgressBar : public UIWindow {
public:
  UIProgressBar(UIWindow *pParent, int x, int y, int nWidth, int nHeight);
  virtual ~UIProgressBar();

  virtual void paint(void);

  void setProgress(int progress);
  void setCurrentOperation(std::string curOp);

private:
  int m_progress;
  std::string m_curOp;
};

/*===========================================================================
  UI button
  ===========================================================================*/
enum UIButtonState { UI_BUTTON_STATE_UNPRESSED, UI_BUTTON_STATE_PRESSED };

enum UIButtonType {
  UI_BUTTON_TYPE_LARGE,
  UI_BUTTON_TYPE_SMALL,
  UI_BUTTON_TYPE_CHECK,
  UI_BUTTON_TYPE_RADIO
};

class UIButton : public UIWindow {
public:
  UIButton() {}
  UIButton(UIWindow *pParent,
           int x = 0,
           int y = 0,
           std::string Caption = "",
           int nWidth = 0,
           int nHeight = 0) {
    initW(pParent, x, y, Caption, nWidth, nHeight);

    m_State = UI_BUTTON_STATE_UNPRESSED;
    m_Type = UI_BUTTON_TYPE_LARGE;
    m_bClicked = false;
    m_bHover = false;
    m_bChecked = false;
  }

  /* Methods */
  virtual void paint(void);
  virtual void mouseLDown(int x, int y);
  virtual void mouseLUp(int x, int y);
  virtual void mouseRDown(int x, int y);
  virtual void mouseRUp(int x, int y);
  virtual void mouseHover(int x, int y);
  virtual void mouseOut(int x, int y);
  virtual bool offerActivation(void);
  virtual bool keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char);
  virtual bool joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton);

  /* Data interface */
  UIButtonState getState(void) { return m_State; }
  void setState(UIButtonState State) { m_State = State; }
  bool isClicked(void) { return m_bClicked; }
  UIButtonType getType(void) { return m_Type; }
  void setType(UIButtonType Type) { m_Type = Type; }
  void setClicked(bool b) { m_bClicked = b; }
  void setChecked(bool b) {
    if (m_Type == UI_BUTTON_TYPE_RADIO && b)
      _UncheckGroup(getGroup());
    m_bChecked = b;
  }
  bool getChecked(void) { return m_bChecked; }

protected:
  UIButtonState m_State;
  UIButtonType m_Type;
  bool m_bHover, m_bClicked, m_bChecked;

  /* Helper methods */
  void _UncheckGroup(int nGroup);

private:
  void toggle();
};

class UIButtonDrawn : public UIButton {
public:
  UIButtonDrawn(UIWindow *pParent,
                const std::string &i_spriteUnpressed,
                const std::string &i_spritePressed,
                const std::string &i_spriteHover,
                int x = 0,
                int y = 0,
                std::string Caption = "",
                int nWidth = 0,
                int nHeight = 0);
  ~UIButtonDrawn();

  virtual void paint();
  void setBorder(int i_border);
  virtual void mouseOut(int x, int y);

private:
  int m_border;
  Texture *m_texturePressed;
  Texture *m_textureUnpressed;
  Texture *m_textureHover;
};

/*===========================================================================
  UI list
  ===========================================================================*/
struct UIListEntry {
  std::vector<std::string> Text;
  void *pvUser;
  bool bFiltered;

  bool bUseOwnProperties;
  Color ownTextColor;
  Color ownSelectedColor;
  Color ownUnSelectedColor;
  int ownXOffset;
  int ownYOffset;
};

class UIList : public UIWindow {
public:
  UIList() {}
  UIList(UIWindow *pParent,
         int x = 0,
         int y = 0,
         std::string Caption = "",
         int nWidth = 0,
         int nHeight = 0);
  virtual ~UIList();

  /* Methods */
  virtual void paint(void);
  virtual void mouseLDown(int x, int y);
  virtual void mouseLDoubleClick(int x, int y);
  virtual void mouseLUp(int x, int y);
  virtual void mouseRDown(int x, int y);
  virtual void mouseRUp(int x, int y);
  virtual void mouseWheelUp(int x, int y);
  virtual void mouseWheelDown(int x, int y);
  virtual void mouseHover(int x, int y);
  virtual bool offerActivation(void);
  virtual bool keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char);
  virtual bool joystickAxisMotion(JoyAxisEvent event);
  virtual bool joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton);
  virtual std::string subContextHelp(int x, int y);

  /* if position != -1, force the entry to this position */
  UIListEntry *addEntry(std::string Text,
                        void *pvUser = NULL,
                        int i_position = -1);
  virtual void clear();

  /* Data interface */
  std::vector<UIListEntry *> &getEntries();
  std::vector<std::string> &getColumns();
  virtual unsigned int getSelected();
  int getRowAtPosition(int x, int y); /* return -1 if none is found */
  int getColumnAtPosition(int x, int y); /* return -1 if none is found */
  void setRealSelected(unsigned int n);
  void setVisibleSelected(unsigned int n);
  void addColumn(std::string Title, int nWidth, const std::string &Help = "");
  void setEnterButton(UIButton *pButton);
  bool isItemActivated();
  void setItemActivated(bool i_value);
  void setHideColumn(int n);
  void unhideAllColumns(void);
  void setSort(bool bSort, int (*f)(void *pvUser1, void *pvUser2) = NULL);
  void setNumeroted(bool bNumeroted);
  std::string getSelectedEntry();

  void randomize();

  bool isClicked(void);
  void setClicked(bool b);

  bool isChanged(void);
  void setChanged(bool b);

  void setFilter(std::string i_filter);
  void checkForFilteredEntries(); // ask the list to check for filtered entries
  // (if you manually set bFiltered to true)

  int nbVisibleItems() const;

private:
  /* Data */
  bool m_bChanged;
  float m_lastRefreshTime;
  bool m_bSort;
  bool m_bNumeroted;
  int (*m_fsort)(void *pvUser1, void *pvUser2);
  std::vector<UIListEntry *> m_Entries;
  std::vector<std::string> m_Columns;
  std::vector<std::string> m_ColumnHelpStrings;
  std::vector<int> m_ColumnWidths;
  unsigned int m_nColumnHideFlags;
  unsigned int m_nRealSelected;
  int m_nVisibleSelected;
  bool m_bItemActivated;
  UIButton *m_pEnterButton; /* if not null this is the "default" button of the
                               list, i.e. the one
                               that gets pressed if the list gets an enter */
  int m_nScroll;
  bool m_bScrollUpPressed, m_bScrollDownPressed;

  bool m_bScrollUpHover, m_bScrollDownHover;

  bool m_bClicked;
  bool m_bScrolling;

  std::string m_filter;
  unsigned int m_filteredItems;

  //
  void adaptRealSelectedOnVisibleEntries();

  /* draw */
  int HeaderHeight();
  int HeaderSubBorderHeight();
  int RowHeight();
  int LineMargeX();
  int LineMargeY();
  int LinesStartX();
  int LinesStartY();
  int LinesWidth();
  int LinesHeight();

  int ScrollBarArrowWidth();
  int ScrollBarArrowHeight();
  int ScrollBarBarWidth();
  int ScrollBarBarHeight();
  int ScrollBarScrollerWidth();
  int ScrollBarScrollerHeight();
  int ScrollBarScrollerStartX();
  int ScrollBarScrollerStartY();
  void setScrollBarScrollerStartY(float y);
  float ScrollNbVisibleItems();
  bool isScrollBarRequired();

  // keyboard/joystick/... events reaction
  void eventGo();
  void eventDown();
  void eventUp();
  void eventLeft();
  void eventRight();
  bool eventJump(int count);
  bool eventJumpAbs(int index, bool allowNegative = false);

  int m_headerHeight;
  int m_headerSubBorderHeight;
  int m_rowHeight;
  int m_lineMargeX, m_lineMargeY;
  int m_scrollBarArrowWidth;
  int m_scrollBarArrowHeight;

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
  UITabView(UIWindow *pParent,
            int x = 0,
            int y = 0,
            std::string Caption = "",
            int nWidth = 0,
            int nHeight = 0) {
    initW(pParent, x, y, Caption, nWidth, nHeight);

    m_nSelected = 0;
    m_bChanged = false;
    m_hideDisabledTabs = false;
  }

  /* Methods */
  virtual void paint(void);
  virtual void mouseLDown(int x, int y);
  virtual void mouseLUp(int x, int y);
  virtual void mouseRDown(int x, int y);
  virtual void mouseRUp(int x, int y);
  virtual void mouseHover(int x, int y);
  virtual std::string subContextHelp(int x, int y);

  void setTabContextHelp(unsigned int nTab, const std::string &s);

  /* Data interface */
  UIWindow *getSelected(void) {
    if (!getChildren().empty() && m_nSelected >= 0 &&
        m_nSelected < getChildren().size())
      return getChildren()[m_nSelected];
    return NULL;
  }
  void setSelected(int n) { m_nSelected = n; }

  bool isChanged(void) { return m_bChanged; }
  void setChanged(bool b) { m_bChanged = b; }

  void selectChildren(unsigned int i);
  void selectChildrenById(const std::string &i_id);

  void setHideDisabledTabs(bool i_value) { m_hideDisabledTabs = i_value; }
  bool hideDisabledTabs() const { return m_hideDisabledTabs; }

private:
  /* Data */
  bool m_bChanged;
  unsigned int m_nSelected;
  std::vector<std::string> m_TabContextHelp;
  bool m_hideDisabledTabs;
};

/*===========================================================================
  UI root
  ===========================================================================*/
enum UIRootMouseEventType {
  UI_ROOT_MOUSE_LBUTTON_DOWN,
  UI_ROOT_MOUSE_LBUTTON_UP,
  UI_ROOT_MOUSE_RBUTTON_DOWN,
  UI_ROOT_MOUSE_RBUTTON_UP,
  UI_ROOT_MOUSE_HOVER,
  UI_ROOT_MOUSE_WHEEL_DOWN,
  UI_ROOT_MOUSE_WHEEL_UP,
  UI_ROOT_MOUSE_DOUBLE_CLICK
};

enum UIRootKeyEventType {
  UI_ROOT_KEY_DOWN,
  UI_ROOT_KEY_UP,
  UI_ROOT_TEXT_INPUT
};

struct UIRootMouseEvent {
  UIRootMouseEventType type;
  int x, y;
  int wheelX, wheelY;
};

struct UIRootKeyEvent {
  UIRootKeyEventType type;
  int nKey;
  SDL_Keymod mod;
  const std::string &utf8Char;
};

struct UIRootActCandidate {
  UIWindow *pWindow;
  int x, y, nWidth, nHeight;
};

class UIRoot : public UIWindow {
public:
  UIRoot(RenderSurface *i_screen);
  ~UIRoot();

  /* Methods */
  virtual void paint(void);
  virtual void mouseLDoubleClick(int x, int y);
  virtual void mouseLDown(int x, int y);
  virtual void mouseLUp(int x, int y);
  virtual void mouseRDown(int x, int y);
  virtual void mouseRUp(int x, int y);
  virtual void mouseHover(int x, int y);
  virtual void mouseWheelUp(int x, int y, Sint16 wheelX, Sint16 wheelY);
  virtual void mouseWheelDown(int x, int y, Sint16 wheelX, Sint16 wheelY);
  virtual bool keyDown(int nKey, SDL_Keymod mod, const std::string &i_utf8Char);
  virtual bool keyUp(int nKey, SDL_Keymod mod, const std::string &i_utf8Char);
  virtual bool textInput(int nKey,
                         SDL_Keymod mod,
                         const std::string &i_utf8Char);
  virtual bool joystickAxisMotion(JoyAxisEvent event);
  virtual bool joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton);

  void deactivate(UIWindow *pWindow);

  void enableContextMenuDrawing(bool b) { m_bShowContextMenu = b; }
  void clearContextHelp(void) { m_CurrentContextHelp = ""; }

  void activateUp(void);
  void activateDown(void);
  void activateLeft(void);
  void activateRight(void);
  void activateNext(void);
  void activatePrevious(void);

  void setApp(GameApp *pApp) {}
  virtual GameApp *getApp(void) { return m_pApp; }
  void dispatchMouseHover();

private:
  /* Data */
  bool m_bShowContextMenu;
  std::string m_CurrentContextHelp;
  GameApp *m_pApp; /* Application */

  /* Helpers */
  bool _RootKeyEvent(UIWindow *pWindow, UIRootKeyEvent Event);

  bool _RootJoystickAxisMotionEvent(UIWindow *pWindow, JoyAxisEvent event);
  bool _RootJoystickButtonDownEvent(UIWindow *pWindow,
                                    Uint8 i_joyNum,
                                    Uint8 i_joyButton);

  bool _RootMouseEvent(UIWindow *pWindow, UIRootMouseEvent Event);
  void _RootPaint(int x, int y, UIWindow *pWindow, UIRect *pRect);
  void _ClipRect(UIRect *pRect, UIRect *pClipWith);
  unsigned int _UpdateActivationMap(UIWindow *pWindow,
                                    UIRootActCandidate *pMap,
                                    unsigned int nNum,
                                    unsigned int nMax);
  void _ActivateByVector(int dx, int dy);
  void _ActivateByStep(int step);
  int _GetActiveIdx(UIRootActCandidate *pMap, unsigned int nNum);

  UIWindow *m_lastHover;
};

#endif
