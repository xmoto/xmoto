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

#include "StateEditWebConfig.h"
#include "../Game.h"
#include "../XMSession.h"
#include "../drawlib/DrawLib.h"
#include "../GameText.h"
#include "StateMessageBox.h"

/* static members */
UIRoot*  StateEditWebConfig::m_sGUI = NULL;

StateEditWebConfig::StateEditWebConfig(bool drawStateBehind,
				       bool updateStatesBehind)
  : StateMenu(drawStateBehind,
	      updateStatesBehind)
{
  m_name    = "StateEditWebConfig";

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("CONFIGURE_WWW_ACCESS");
  }
}

StateEditWebConfig::~StateEditWebConfig()
{
}

void StateEditWebConfig::enter()
{
  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;
  updateGUI();

  if(XMSession::instance()->webConfAtInit()) {
    // show the message box
    StateMessageBox* v_msgboxState = new StateMessageBox(this, std::string(GAMETEXT_ALLOWINTERNETCONN),
							 UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setMsgBxId("EDITWEBCONF");
    StateManager::instance()->pushState(v_msgboxState);
  }

  StateMenu::enter();
}

void StateEditWebConfig::checkEvents()
{
  /* Get some pointers */
  UIButton *pDirectConn = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:DIRECTCONN"));
  UIButton *pHTTPConn   = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:HTTPPROXY"));
  UIButton *pSOCKS4Conn = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:SOCKS4PROXY"));
  UIButton *pSOCKS5Conn = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:SOCKS5PROXY"));
  UIButton *pConnOK     = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:PROXYOK"));
  UIEdit   *pServer     = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:SERVEREDIT"));
  UIEdit   *pPort       = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:PORTEDIT"));    
  UIEdit   *pLogin      = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:LOGINEDIT")); 
  UIEdit   *pPassword   = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:PASSWORDEDIT"));     

  if(pDirectConn->isClicked() || pHTTPConn->isClicked() || pSOCKS4Conn->isClicked() || pSOCKS5Conn->isClicked()) {
    pDirectConn->setClicked(false);
    pHTTPConn->setClicked(false);
    pSOCKS4Conn->setClicked(false);
    pSOCKS5Conn->setClicked(false);
    updateGUIRights();
  }

  /* OK button pressed? */
  if(pConnOK->isClicked()) {
    pConnOK->setClicked(false);

    /* Save settings */
    std::string ProxyType = "";
    if(pHTTPConn->getChecked())
      ProxyType = "HTTP";
    else if(pSOCKS4Conn->getChecked())
      ProxyType = "SOCKS4";
    else if(pSOCKS5Conn->getChecked())
      ProxyType = "SOCKS5";
    else
      ProxyType = "";

    XMSession::instance()->proxySettings()->setType(ProxyType);
    XMSession::instance()->proxySettings()->setPort(atoi(pPort->getCaption().c_str()));
    XMSession::instance()->proxySettings()->setServer(pServer->getCaption());
    XMSession::instance()->proxySettings()->setAuthentification(pLogin->getCaption(), pPassword->getCaption());
    XMSession::instance()->markProxyUpdated();

    XMSession::instance()->setWWW(true);
    StateManager::instance()->sendAsynchronousMessage("CONFIGURE_WWW_ACCESS");

    m_requestForEnd = true;
  }
}

void StateEditWebConfig::clean()
{
  if(StateEditWebConfig::m_sGUI != NULL) {
    delete StateEditWebConfig::m_sGUI;
    StateEditWebConfig::m_sGUI = NULL;
  }
}

void StateEditWebConfig::createGUIIfNeeded(RenderSurface* i_screen)
{
  UIStatic *pSomeText;
  UIFrame  *v_frame;

  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      i_screen->getDispWidth(),
		      i_screen->getDispHeight());

  /* Initialize internet connection configurator */
  int x = i_screen->getDispWidth()/2-206;
  int y = i_screen->getDispHeight()/2-385/2;
  std::string caption = "";
  int nWidth  = 412;
  int nHeight = 425;

  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight);
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);           
  v_frame->setID("EDITWEBCONF_FRAME");

  UIStatic *pWebConfEditorTitle = new UIStatic(v_frame,0,0,GAMETEXT_INETCONF,400,50);
  pWebConfEditorTitle->setFont(drawLib->getFontMedium());
   
#if defined(WIN32)
  /* I don't expect a windows user to know what an environment variable is */
  #define DIRCONNTEXT std::string(GAMETEXT_DIRECTCONN).c_str()
#else
  #define DIRCONNTEXT (std::string(GAMETEXT_DIRECTCONN) + " / " + std::string(GAMETEXT_USEENVVARS)).c_str()
#endif

  int radioButtonsGroup = 16023;

  UIButton *pConn1 = new UIButton(v_frame,25,60,DIRCONNTEXT,(v_frame->getPosition().nWidth-50),28);
  pConn1->setType(UI_BUTTON_TYPE_RADIO);
  pConn1->setID("DIRECTCONN");
  pConn1->setFont(drawLib->getFontSmall());
  pConn1->setGroup(radioButtonsGroup);
  pConn1->setContextHelp(CONTEXTHELP_DIRECTCONN);

  UIButton *pConn2 = new UIButton(v_frame,25,88,GAMETEXT_USINGHTTPPROXY,(v_frame->getPosition().nWidth-160),28);
  pConn2->setType(UI_BUTTON_TYPE_RADIO);
  pConn2->setID("HTTPPROXY");
  pConn2->setFont(drawLib->getFontSmall());
  pConn2->setGroup(radioButtonsGroup);
  pConn2->setContextHelp(CONTEXTHELP_HTTPPROXY);

  UIButton *pConn3 = new UIButton(v_frame,25,116,GAMETEXT_USINGSOCKS4PROXY,(v_frame->getPosition().nWidth-160),28);
  pConn3->setType(UI_BUTTON_TYPE_RADIO);
  pConn3->setID("SOCKS4PROXY");
  pConn3->setFont(drawLib->getFontSmall());
  pConn3->setGroup(radioButtonsGroup);
  pConn3->setContextHelp(CONTEXTHELP_SOCKS4PROXY);

  UIButton *pConn4 = new UIButton(v_frame,25,144,GAMETEXT_USINGSOCKS5PROXY,(v_frame->getPosition().nWidth-160),28);
  pConn4->setType(UI_BUTTON_TYPE_RADIO);
  pConn4->setID("SOCKS5PROXY");
  pConn4->setFont(drawLib->getFontSmall());
  pConn4->setGroup(radioButtonsGroup);
  pConn4->setContextHelp(CONTEXTHELP_SOCKS5PROXY);
    
  UIButton *pConnOKButton = new UIButton(v_frame,(v_frame->getPosition().nWidth-160)+28,(v_frame->getPosition().nHeight-68),GAMETEXT_OK,115,57);
  pConnOKButton->setFont(drawLib->getFontSmall());
  pConnOKButton->setType(UI_BUTTON_TYPE_SMALL);
  pConnOKButton->setID("PROXYOK");
  pConnOKButton->setContextHelp(CONTEXTHELP_OKPROXY);
    
  UIFrame *pSubFrame = new UIFrame(v_frame,25,185,"",(v_frame->getPosition().nWidth-50),(v_frame->getPosition().nHeight-185-75));
  pSubFrame->setStyle(UI_FRAMESTYLE_TRANS);
  pSubFrame->setID("SUBFRAME");    
    
  pSomeText = new UIStatic(pSubFrame,10,25,std::string(GAMETEXT_PROXYSERVER) + ":",120,25);
  pSomeText->setFont(drawLib->getFontSmall());    
  pSomeText->setHAlign(UI_ALIGN_RIGHT);

  UIEdit *pProxyServerEdit = new UIEdit(pSubFrame,135,25,"",190,25);
  pProxyServerEdit->setFont(drawLib->getFontSmall());
  pProxyServerEdit->setID("SERVEREDIT");
  pProxyServerEdit->setContextHelp(CONTEXTHELP_PROXYSERVER);

  pSomeText = new UIStatic(pSubFrame,10,55,std::string(GAMETEXT_PORT) + ":",120,25);
  pSomeText->setFont(drawLib->getFontSmall());    
  pSomeText->setHAlign(UI_ALIGN_RIGHT);

  UIEdit *pProxyPortEdit = new UIEdit(pSubFrame,135,55,"",50,25);
  pProxyPortEdit->setFont(drawLib->getFontSmall());
  pProxyPortEdit->setID("PORTEDIT");
  pProxyPortEdit->setContextHelp(CONTEXTHELP_PROXYPORT);

  pSomeText = new UIStatic(pSubFrame,10,85,std::string(GAMETEXT_LOGIN) + ":",120,25);
  pSomeText->setFont(drawLib->getFontSmall());    
  pSomeText->setHAlign(UI_ALIGN_RIGHT);

  UIEdit *pProxyLoginEdit = new UIEdit(pSubFrame,135,85,"",190,25);
  pProxyLoginEdit->setFont(drawLib->getFontSmall());
  pProxyLoginEdit->setID("LOGINEDIT");
  pProxyLoginEdit->setContextHelp(CONTEXTHELP_PROXYLOGIN);

  pSomeText = new UIStatic(pSubFrame,10,115,std::string(GAMETEXT_PASSWORD) + ":",120,25);
  pSomeText->setFont(drawLib->getFontSmall());    
  pSomeText->setHAlign(UI_ALIGN_RIGHT);

  UIEdit *pProxyPasswordEdit = new UIEdit(pSubFrame,135,115,"",190,25);
  pProxyPasswordEdit->setFont(drawLib->getFontSmall());
  pProxyPasswordEdit->setID("PASSWORDEDIT");
  pProxyPasswordEdit->setContextHelp(CONTEXTHELP_PROXYPASSWORD);
}

void StateEditWebConfig::updateGUIRights() {
  UIButton *pDirectConn = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:DIRECTCONN"));
  UIEdit   *pServer     = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:SERVEREDIT"));
  UIEdit   *pPort       = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:PORTEDIT"));    
  UIEdit   *pLogin      = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:LOGINEDIT")); 
  UIEdit   *pPassword   = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:PASSWORDEDIT"));     

  /* Direct connection selected? If so, no need to enabled proxy editing */
  if(pDirectConn->getChecked()) {
    pServer->enableWindow(false);
    pPort->enableWindow(false);
    pLogin->enableWindow(false);
    pPassword->enableWindow(false);
  }            
  else {
    pServer->enableWindow(true);
    pPort->enableWindow(true);
    pLogin->enableWindow(true);
    pPassword->enableWindow(true);
  }
}

void StateEditWebConfig::updateGUI()
{
  /* Get some pointers */
  UIButton *pDirectConn = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:DIRECTCONN"));
  UIButton *pHTTPConn   = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:HTTPPROXY"));
  UIButton *pSOCKS4Conn = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:SOCKS4PROXY"));
  UIButton *pSOCKS5Conn = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:SOCKS5PROXY"));
  UIButton *pConnOK     = reinterpret_cast<UIButton *>(m_GUI->getChild("EDITWEBCONF_FRAME:PROXYOK"));
  UIEdit   *pServer     = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:SERVEREDIT"));
  UIEdit   *pPort       = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:PORTEDIT"));
  UIEdit   *pLogin      = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:LOGINEDIT"));
  UIEdit   *pPassword   = reinterpret_cast<UIEdit *>(m_GUI->getChild("EDITWEBCONF_FRAME:SUBFRAME:PASSWORDEDIT"));

  pDirectConn->setChecked(false);
  pHTTPConn->setChecked(false);
  pSOCKS4Conn->setChecked(false);
  pSOCKS5Conn->setChecked(false);

  /* Read config */
  pServer->setCaption(XMSession::instance()->proxySettings()->getServer());
  char cBuf[256] = "";
  int  n = XMSession::instance()->proxySettings()->getPort();
  if(n > 0)
    snprintf(cBuf, 256, "%d", n);
  pPort->setCaption(cBuf);
  pLogin->setCaption(XMSession::instance()->proxySettings()->getAuthentificationUser());
  pPassword->setCaption(XMSession::instance()->proxySettings()->getAuthentificationPassword());

  std::string proxyType = XMSession::instance()->proxySettings()->getTypeStr();
  if(proxyType == "HTTP")
    pHTTPConn->setChecked(true);
  else if(proxyType == "SOCKS4")
    pSOCKS4Conn->setChecked(true);
  else if(proxyType == "SOCKS5")
    pSOCKS5Conn->setChecked(true);
  else
    pDirectConn->setChecked(true);

  /* Make sure OK button is activated */
  pConnOK->makeActive();

  updateGUIRights();
}
void StateEditWebConfig::sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input)
{
  /* The yes/no box open? */
  if(i_id == "EDITWEBCONF") {
    switch(i_button){
    case UI_MSGBOX_YES:
      /* Show the actual web config editor */
      XMSession::instance()->setWWW(true);
      break;
    case UI_MSGBOX_NO:
      /* No internet connection thank you */
      XMSession::instance()->setWWW(false);
      StateManager::instance()->sendAsynchronousMessage("CHANGE_WWW_ACCESS");
      m_requestForEnd = true;
      break;
    default:
      break;
    }
    XMSession::instance()->setWebConfAtInit(false);
  } else {
    StateMenu::sendFromMessageBox(i_id, i_button, i_input);
  }
}
