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
 *  Editor application. Beware, this code is rather clumsy and unstructured.
 *  As usual, I was shocked by the level of complexity of an editor :)
 */
#define XMOTO_EDITOR
#include "BuildConfig.h"
#include "Editor.h"
#include "VFileIO.h"
#include "VBezier.h"
#include "Packager.h"

#if defined(_DEBUG)
  #define XMOTOCOMMAND "xmotod"
#else
  #define XMOTOCOMMAND "xmoto"
#endif 

namespace vapp {

  /*============================================================================
  Editor app constructor
  ============================================================================*/
  EditorApp::EditorApp() {
    m_pLevelSrc=NULL; 
    m_bLeftButtonDown=false; 
    m_pEntityToCopy=NULL; 
    m_bJustPackage=false;
#if defined(SUPPORT_WEBACCESS)
    m_themeChoicer = new ThemeChoicer(NULL,NULL);         
#else
    m_themeChoicer = new ThemeChoicer();
#endif
  }
  
  /*============================================================================
  Draw graphics
  ============================================================================*/
  void EditorApp::drawFrame(void) {
    /* Update mouse */
    m_bLeftButtonClick=false;
    if( SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(1) ) {
      /* Left button is down */
      if(!m_bLeftButtonDown) m_bLeftButtonClick=true;
      m_bLeftButtonDown=true;
    }
    else m_bLeftButtonDown=false;
  
    /* Read selectors */
    switch(m_SnapSelect.getSelIdx()) {
      case 0: m_nGridSnap = 1; break;
      case 1: m_nGridSnap = 10; break;
      case 2: m_nGridSnap = 100; break;
      default: m_nGridSnap = 0; break;
    }
    
    switch(m_SelStyleSelect.getSelIdx()) {
      case 0: m_SelMode = SM_VERTICES; break;
      case 1: m_SelMode = SM_EDGES; break;
      case 2: m_SelMode = SM_BLOCKS; break;
      case 3: m_SelMode = SM_ENTITIES; break;
    }
    
    /* Update cursor position */
    m_PrevCursor = m_Cursor;
    int nX,nY;
    SDL_GetMouseState(&nX,&nY);
    if(nX >= m_ViewportPos.x && nY >= m_ViewportPos.y &&
       nX < m_ViewportPos.x+m_ViewportSize.x && nY < m_ViewportPos.y+m_ViewportSize.y) {
      m_Cursor = Vector2f(m_ViewMin.x + (((float)nX-m_ViewportPos.x) / (m_ViewportSize.x))*(m_ViewMax.x - m_ViewMin.x),
                          m_ViewMax.y - (((float)nY-m_ViewportPos.y) / (m_ViewportSize.y))*(m_ViewMax.y - m_ViewMin.y));
      m_bCursorInViewport = true;   
      
      if(_IsCtrlDown()) {
        _EdgeSnapCursor();
      }
      else if(_IsAltDown()) {
        _EntitySnapCursor();
      }
      else {        
        if(m_nGridSnap!=0) {      
          m_Cursor.x *= (float)m_nGridSnap;
          m_Cursor.y *= (float)m_nGridSnap;
          float xfd = m_Cursor.x - floor(m_Cursor.x);
          float xcd = ceil(m_Cursor.x) - m_Cursor.x;
          float yfd = m_Cursor.y - floor(m_Cursor.y);
          float ycd = ceil(m_Cursor.y) - m_Cursor.y;
          if(xfd < xcd) m_Cursor.x = floor(m_Cursor.x);
          else m_Cursor.x = ceil(m_Cursor.x);
          if(yfd < ycd) m_Cursor.y = floor(m_Cursor.y);
          else m_Cursor.y = ceil(m_Cursor.y);
          m_Cursor.x /= (float)m_nGridSnap;
          m_Cursor.y /= (float)m_nGridSnap;
        }      
      }
//      if(m_CurState == ES_DEFAULT || m_CurState == ES_CREATING_NEW_BLOCK) SDL_ShowCursor(0);
    }
    else {
      m_bCursorInViewport = false;
//      if(m_CurState == ES_DEFAULT || m_CurState == ES_CREATING_NEW_BLOCK) SDL_ShowCursor(1);
    }
    
    /* Update misc. */
    if(nX >= m_TextureBrowserPos.x && nY >= m_TextureBrowserPos.y &&
       nX < m_TextureBrowserPos.x+m_TextureBrowserSize.x && nY < m_TextureBrowserPos.y+m_TextureBrowserSize.y) {
      m_bTextureBrowserMouseHover = true;
    }
    else m_bTextureBrowserMouseHover = false;
            
    /* Draw graphics */
    glClearColor(0.2f,0.2f,0.5f,0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawBox(m_MainMenuAreaPos,m_MainMenuAreaPos+m_MainMenuAreaSize,2,MAKE_COLOR(40,40,60,255),MAKE_COLOR(0,0,0,255));
    drawBox(m_ViewportPos-Vector2f(1,1),m_ViewportPos+m_ViewportSize+Vector2f(2,2),1,MAKE_COLOR(160,160,160,255),MAKE_COLOR(255,255,255,255));
    drawBox(m_TextureBrowserPos,m_TextureBrowserPos+m_TextureBrowserSize,0,MAKE_COLOR(0,0,0,255),0);
    
    /* Draw menus */
    m_pMainMenu->drawMenu( this,m_MainMenuAreaPos + Vector2f(2,2) );
    drawText(m_MainMenuAreaPos + Vector2f(2,240),"Avail. entities:",0,MAKE_COLOR(200,200,200,255));
    m_pEntityMenu->drawMenu( this,m_MainMenuAreaPos + Vector2f(10,252) );
        
    /* Draw selectors */
    m_SnapSelect.drawSimpleSelector( this,Vector2f(0,getDispHeight()-12) );
    m_SelStyleSelect.drawSimpleSelector( this,Vector2f(m_SnapSelect.getWidth()+16,getDispHeight()-12) );
    
    /* Info */
    char cBuf[256];
    if(m_nGridSnap>10000) sprintf(cBuf,"%.5f,%.5f",m_Cursor.x,m_Cursor.y);
    else if(m_nGridSnap>1000) sprintf(cBuf,"%.4f,%.4f",m_Cursor.x,m_Cursor.y);
    else if(m_nGridSnap>100) sprintf(cBuf,"%.3f,%.3f",m_Cursor.x,m_Cursor.y);
    else if(m_nGridSnap>10) sprintf(cBuf,"%.2f,%.2f",m_Cursor.x,m_Cursor.y);
    else if(m_nGridSnap>1) sprintf(cBuf,"%.1f,%.1f",m_Cursor.x,m_Cursor.y);
    else if(m_nGridSnap>0) sprintf(cBuf,"%.0f,%.0f",m_Cursor.x,m_Cursor.y);
    else sprintf(cBuf,"%f,%f",m_Cursor.x,m_Cursor.y);
    drawText(Vector2f(0,0),std::string("CUR:") + std::string(cBuf),0,MAKE_COLOR(255,255,255,255));
    
    if(_IsAltDown()) drawText(Vector2f(300,0),"ENTITY-SNAP!");
    else if(_IsCtrlDown()) drawText(Vector2f(300,0),"EDGE-SNAP!");
    
    /* Viewport */
    //glScissor(m_ViewportPos.x,((float)getDispHeight()) - (m_ViewportPos.y+m_ViewportSize.y)-1.0,
    //          m_ViewportSize.x+1.0,m_ViewportSize.y+1.0);
    scissorGraphics(m_ViewportPos.x,m_ViewportPos.y,m_ViewportSize.x,m_ViewportSize.y);
    glEnable(GL_SCISSOR_TEST);
    viewDrawGrid();
    glDisable(GL_SCISSOR_TEST);
    //glScissor(0,0,(float)getDispWidth(),(float)getDispHeight());
    scissorGraphics(0,0,getDispWidth(),getDispHeight());
    
    /* Draw log */
    drawBox(Vector2f(m_ViewportPos.x,m_ViewportPos.y+m_ViewportSize.y+4),
            Vector2f(m_ViewportPos.x+m_ViewportSize.x+1,getDispHeight()-14),
            1,MAKE_COLOR(40,40,60,255),MAKE_COLOR(160,160,160,255));
		m_Log.drawWindow(this,            
		                 Vector2f(m_ViewportPos.x+1,m_ViewportPos.y+m_ViewportSize.y+4),
		                 Vector2f(m_ViewportPos.x+m_ViewportSize.x+1,getDispHeight()-14) -
		                 Vector2f(m_ViewportPos.x+1,m_ViewportPos.y+m_ViewportSize.y+4),
		                 0,MAKE_COLOR(160,160,160,255));
		                 
    /* Draw texture browser box */
    _DrawTextureBrowser();
    
    /* State dependant message? */
    if(m_CurState == ES_MOVING_LIMITS) {
      viewMessage("Click and drag limits, press ESC when finished");
    }
    else if(m_CurState == ES_CREATING_NEW_BLOCK) {
      viewMessage("Place vertices of new block, click first vertex to finish,    \n"
                  "press BACKSPACE to cancel last vertex ESC to cancel everything");
    }
    else if(m_CurState == ES_MOVING_SELECTION) {
      viewMessage("Release button to finish movement, press ESC to cancel");
    }
    else if(m_CurState == ES_CREATING_NEW_ENTITY) {
			viewMessage(std::string("Click to place entity \"") + m_NewEntityID + std::string("\", press ESC to cancel"));
    }
    else if(m_CurState == ES_COPYING_ENTITY) {
      if(m_pEntityToCopy == NULL) {
        m_CurState = ES_DEFAULT;
      }
      else 
        viewMessage(std::string("Click to place a copy of \"") + m_pEntityToCopy->Id() + std::string("\", press ESC to cancel"));
    }
  }
  
  /*============================================================================
  Init
  ============================================================================*/
  void EditorApp::userInit(void) {
    if(m_bJustPackage) {
      quit();
      return;  
    }
    
    /* No graphics? */
    if(isNoGraphics())
      throw Exception("editor requires graphics");
  
		/* Load editor.dat (and possibly friends) */
		m_EntityTable.loadFile("editor.dat");
		m_EntityTable.consoleDump();
    
    /* Determine position and size of main GUI elements */
    m_MainMenuAreaSize = Vector2f(160,getDispHeight()-132);
    m_MainMenuAreaPos  = Vector2f(getDispWidth()-m_MainMenuAreaSize.x,0);
    
    m_TextureBrowserSize = Vector2f(132,132);
    m_TextureBrowserPos = Vector2f(getDispWidth()-m_MainMenuAreaSize.x,getDispHeight()-132);
    
    m_nLogLines = 6; /* per default show lines of the log */
    
    m_ViewportSize = Vector2f(getDispWidth() - m_MainMenuAreaSize.x - 5,
                              getDispHeight() - 27 - (m_nLogLines*12+6));
    m_ViewportPos  = Vector2f(2,13);
    
    /* Default stuff */
    m_bTextureBrowserMouseHover = false;
        
    /* Snap */
    m_nGridSnap = 1;
    
    /* Create menu */
    m_pMainMenu = new EditorMainMenu( this );
    m_pMainMenu->addItem("New Level         F1");   /* 0 */
    m_pMainMenu->addItem("Save Level        F2");   /* 1 */
    m_pMainMenu->addItem("Load Level        F3");   /* 2 */
    m_pMainMenu->addItem("Play Level        F4");   /* 3 */
    m_pMainMenu->addItem("--------------------");
    m_pMainMenu->addItem("Move Limits       F5");   /* 5 */
    m_pMainMenu->addItem("Create Block      F6");   /* 6 */
    m_pMainMenu->addItem("Edit Sel. Entity  F7");		/* 7 */
    m_pMainMenu->addItem("Level Properties  F8");   /* 8 */
    m_pMainMenu->addItem("Edit Edge FX      F9");   /* 9 */
    m_pMainMenu->addItem("Smooth Edges     F10");   /* 10 */
    m_pMainMenu->addItem("Delete Selection DEL");   /* 11 */
    m_pMainMenu->addItem("Tgl. Blk BG   CTRL-B");   /* 12 */
    //m_pMainMenu->addItem("Tgl. Blk Watr CTRL-W");   /* 12 */
    m_pMainMenu->addItem("Copy Entity   CTRL-C");   /* 13 */
    m_pMainMenu->addItem("--------------------");
    m_pMainMenu->addItem("Exit          CTRL-X");   /* 15 */
    
    /* Create entity menu */
    m_pEntityMenu = new EditorEntityMenu( this );
    for(int i=0;i<m_EntityTable.getTypes().size();i++) {
      char cBuf[256],cBuf2[256];
      sprintf(cBuf,"%-13s",m_EntityTable.getTypes()[i]->ID.c_str());
      if(strlen(cBuf) > 13) cBuf[13]='\0';
      sprintf(cBuf2,"%sCTRL-%d",cBuf,i+1);            
			m_pEntityMenu->addItem(cBuf2);
    }
    
    /* Create snap select menu */
    m_SnapSelect.setName("SNAP:");
    m_SnapSelect.addItem("1");
    m_SnapSelect.addItem("1/10");
    m_SnapSelect.addItem("1/100");
    m_SnapSelect.addItem("OFF");
    
    /* Create selection menu */
    m_SelStyleSelect.setName("SEL:");
    m_SelStyleSelect.addItem("VERTICES");
    m_SelStyleSelect.addItem("EDGES");
    m_SelStyleSelect.addItem("BLOCKS");
    m_SelStyleSelect.addItem("ENTITIES");

    /* Load level or create blank? */
    if(m_LevelToInitLoad != "") {
      std::string Path = std::string("Levels/") + m_LevelToInitLoad;			  
      m_Log.msg("Loading level from '%s'...",Path.c_str());         
      m_pLevelSrc = new Level();
      m_pLevelSrc->setFileName( Path );
      try {
        m_pLevelSrc->loadXML();
        resetView();
        setState(ES_DEFAULT);
      }
      catch(Exception &e) {
        m_Log.msg("File '%s' failed to load, creating new instead.",m_LevelToInitLoad.c_str());
        m_Log.msg("If you're sure that the level SHOULD work, DON'T save now, exit instead!");
        m_Log.msg("(otherwise you might overwrite a valid level)");
        
        delete m_pLevelSrc;
        m_pLevelSrc = NULL;
        
        _NewLevel();
        m_pLevelSrc->setFileName( Path );
      }
    }        
    else {
      _NewLevel();
    }
    
    /* Load default theme */    
    m_theme.load(m_themeChoicer->getFileName(THEME_DEFAULT_THEMENAME));

    /* Output log message */
    m_Log.msg("Ready. (X-Moto version %s)",getVersionString().c_str());		
  }
  
  /*============================================================================
  Shutdown
  ============================================================================*/
  void EditorApp::userShutdown(void) {
    if(m_pLevelSrc != NULL) delete m_pLevelSrc;
    delete m_pEntityMenu;
    delete m_pMainMenu;
  }
  
  /*============================================================================
  Mouse button down event handler
  ============================================================================*/
  void EditorApp::mouseDown(int nButton) {
    switch(nButton) {
      case SDL_BUTTON_WHEELDOWN:  
        /* Mouse wheel down -- zoom out */
        zoomView(1.04f);
        break;
      case SDL_BUTTON_WHEELUP:  
        /* Mouse wheel up -- zoom in */
        zoomView(0.96f);
        break;
      case SDL_BUTTON_RIGHT:
				/* Scroll! */
				setViewPoint(m_Cursor);
				break;
      case SDL_BUTTON_LEFT:
        if(m_CurState == ES_DEFAULT) {
          /* Default state...  */
          if(m_bCursorInViewport) {
            /* Is the guy clicking on something? */
            if(_AnythingAtPoint(m_Cursor,true) != CT_NOTHING) {
              /* Start moving active selection instead */
              setState(ES_MOVING_SELECTION);
            }
            else {
              /* No, start selection */
              setState(ES_SELECTING);
            }
          }
        }
        /* Are we placing an entity? - then do it here */
        else if(m_CurState == ES_CREATING_NEW_ENTITY) {
					if(m_bCursorInViewport) {
						_CreateEntityAtPos(m_NewEntityID,m_Cursor);
						setState(ES_DEFAULT);
					}
        }
        /* Place an entity copy here */
        else if(m_CurState == ES_COPYING_ENTITY) {
          if(m_bCursorInViewport) {
            _CopyEntityAtPos(m_pEntityToCopy,m_Cursor);
          }
        }
        /* Are we creating a new block? - if so, place a vertex here */
        else if(m_CurState == ES_CREATING_NEW_BLOCK) {
          if(m_bCursorInViewport) {
            /* Is this the first vertex? */
            if(m_NewBlockX.size() > 2 && fabs(m_Cursor.x - m_NewBlockX[0]) < 0.3 && fabs(m_Cursor.y - m_NewBlockY[0]) < 0.3) {
              /* Finish creation */
              Block *pBlock = new Block();
              m_pLevelSrc->Blocks().push_back(pBlock);
              pBlock->fPosX = _GetAvg(m_NewBlockX);
              pBlock->fPosY = _GetAvg(m_NewBlockY);
              pBlock->fTextureScale = 1.0f;              
              pBlock->ID = _CreateBlockName();
              pBlock->Texture = "default";
              
              //float ff = 0;
              
              for(int i=0;i<m_NewBlockX.size();i++) {
                LevelBlockVertex *pVertex = new LevelBlockVertex;
                pBlock->Vertices.push_back( pVertex );
                pVertex->fX = m_NewBlockX[i] - pBlock->fPosX;
                pVertex->fY = m_NewBlockY[i] - pBlock->fPosY;
                pVertex->fTX = m_NewBlockX[i]; 
                pVertex->fTY = m_NewBlockY[i]; 
                pVertex->r = 255;
                pVertex->g = 255;
                pVertex->b = 255;
                pVertex->a = 255;
                pVertex->bSelected = false;
              }
              
              setState(ES_DEFAULT);
            }
            else {
              m_NewBlockX.push_back( m_Cursor.x );
              m_NewBlockY.push_back( m_Cursor.y );
            }
          }
        }
        break;
    }
  }

  /*============================================================================
  Mouse button up event handler
  ============================================================================*/
  void EditorApp::mouseUp(int nButton) {
    switch(nButton) {
      case SDL_BUTTON_LEFT:
        /* If ES_SELECTING -- end selection */
        if(m_CurState == ES_SELECTING) {  
          _SelectByBox();
          m_CurState = ES_DEFAULT; /* back to default state */          
        }
        else if(m_CurState == ES_MOVING_SELECTION) {
          /* Finish moving */
          _FinishMovement( m_Cursor - m_MStart );
          m_CurState = ES_DEFAULT; /* back to default state */          
        }
        else {  
          /* Inside texture browser? */
          if(m_bTextureBrowserMouseHover && m_SelMode == SM_BLOCKS) { /* TODO: also check if sel is empty */
            /* Open tool */
            TextureSelectionTool Sel;
            std::vector<Texture *> TexList;
            
	          std::vector<Sprite*> v_sprites = m_theme.getSpritesList();
	          Texture* v_texture;
	          for(int i=0; i<v_sprites.size(); i++) {
	            if(v_sprites[i]->getType() == SPRITE_TYPE_TEXTURE) {
                v_texture = v_sprites[i]->getTexture();
                if(v_texture != NULL) {
	                TexList.push_back(v_texture);
                }
	            }
	          }
	                      
            Vector2f A=Vector2f(17,17);
            Vector2f B=Vector2f(getDispWidth()-66,getDispHeight()-27);
            //glScissor(A.x,getDispHeight() - B.y-1,B.x-A.x+1,B.y-A.y+1);
            scissorGraphics(A.x,A.y,(B.x - A.x),(B.y - A.y));
            glClearColor(0.1,0.1,0.3,0);
            glEnable(GL_SCISSOR_TEST);
            Sel.setDims(A,B);
            Sel.setList(TexList);
            Sel.run(this);
            glDisable(GL_SCISSOR_TEST);
            
            /* Okay, what happened? */
            if(Sel.getSelectedIdx() >= 0) {
              /* Nice, assign this texture to selection */
              _AssignTextureToSelection( TexList[Sel.getSelectedIdx()] );
            }
          }
        }
        break;
    }
  }
  
  /*============================================================================
  Key down event handler
  ============================================================================*/
  void EditorApp::keyDown(int nKey,int nChar) {
    switch(nKey) {
      case SDLK_F1:
        setState(ES_CREATING_NEW_LEVEL);
        break;
      case SDLK_F2:
        setState(ES_SAVING_LEVEL);
        break;
      case SDLK_F3:
        setState(ES_LOADING_LEVEL);
        break;
      case SDLK_F4:
        setState(ES_PLAYING_LEVEL);
        break;
      case SDLK_F5:
        setState(ES_MOVING_LIMITS);
        break;
      case SDLK_F6:
        setState(ES_CREATING_NEW_BLOCK);
        break;
      case SDLK_F7:
        setState(ES_EDITING_SELECTED_ENTITY);
        break;
      case SDLK_F8:
        setState(ES_EDITING_LEVEL_PROPS);
        break;
      case SDLK_F9:
        setState(ES_EDITING_EDGE_EFFECT);
        break;
      case SDLK_F10:
        setState(ES_SMOOTHING_EDGES);
        break;
      case SDLK_b:
        if(_IsCtrlDown()) {
          setState(ES_TOGGLE_BG_BLOCK);
        }
        break;
      case SDLK_c:
        if(_IsCtrlDown()) {
          setState(ES_COPYING_ENTITY);
        }
        break;
      case SDLK_DELETE:
        setState(ES_DELETING_SELECTION);
        break;
      case SDLK_x:
        if(_IsCtrlDown())
          quit();
        break;
      case SDLK_1:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,0 );
        }
        else
          m_SelStyleSelect.setSelIdx(0);
        break;      
      case SDLK_2:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,1 );
        }
        else
          m_SelStyleSelect.setSelIdx(1);
        break;      
      case SDLK_3:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,2 );
        }
        else
          m_SelStyleSelect.setSelIdx(2);
        break;      
      case SDLK_4:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,3 );
        }
        else
          m_SelStyleSelect.setSelIdx(3);
        break;      
      case SDLK_5:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,4 );
        }
        break;
      case SDLK_6:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,5 );
        }
        break;
      case SDLK_7:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,6 );
        }
        break;
      case SDLK_8:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,7 );
        }
        break;
      case SDLK_9:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,8 );
        }
        break;
      case SDLK_0:
        if(_IsCtrlDown()) {
      		setState( ES_CREATING_NEW_ENTITY,9 );
        }
        break;
      case SDLK_q:
        m_SnapSelect.setSelIdx(0);
        break;      
      //case SDLK_w:
      //  if(_IsCtrlDown()) {
      //    setState( ES_TOGGLE_WATER_BLOCK );
      //  }
      //  else
      //    m_SnapSelect.setSelIdx(1);
      //  break;      
      case SDLK_e:
        m_SnapSelect.setSelIdx(2);
        break;      
      case SDLK_r:
        m_SnapSelect.setSelIdx(3);
        break;      
      case SDLK_ESCAPE:
        /* What state? */
        if(m_CurState == ES_MOVING_LIMITS) {
          /* We are moving level limits -- quit it */
          m_CurState = ES_DEFAULT;
        }
        if(m_CurState == ES_COPYING_ENTITY) {
          /* Cancel entity copying */
          m_CurState = ES_DEFAULT;
        }
        else if(m_CurState == ES_CREATING_NEW_BLOCK) {
          /* Cancel block creation */
          m_CurState = ES_DEFAULT;
        }
        else if(m_CurState == ES_MOVING_SELECTION) {
          /* Cancel stuff movin' */
          m_CurState = ES_DEFAULT;
        }
        else if(m_CurState == ES_CREATING_NEW_ENTITY) {
					/* Don't create that entity */
					m_CurState = ES_DEFAULT;
        }
        break;
      case SDLK_BACKSPACE:
        /* What state? */
        if(m_CurState == ES_CREATING_NEW_BLOCK) {
          /* Cancel last vertex placement */
          if(m_NewBlockX.size() > 0) {
            m_NewBlockX.pop_back();
            m_NewBlockY.pop_back();
          }
        }
        break;
      case SDLK_z:  
        zoomView(1.04f);
        break;
      case SDLK_a:  
        zoomView(0.96f);
        break;
    }
  }
  
  /*============================================================================
  Key up event handler
  ============================================================================*/
  void EditorApp::keyUp(int nKey) {
  }
  
  /*============================================================================
  Parse additional editor command-line arguments
  ============================================================================*/
  void EditorApp::parseUserArgs(std::vector<std::string> &UserArgs) {
    for(int i=0;i<UserArgs.size();i++) {
      if(UserArgs[i][0] != '-') {
        m_LevelToInitLoad = UserArgs[i];
      }
      else {
        /* Should we invoke the packager? */
        if(UserArgs[i] == "-pack") {
          Packager::go();
          exit(0); /* leaks memory, but who cares? :) */
        }
        if(UserArgs[i] == "-unpack") {
          std::string BinFile = "xmoto.bin";
          if(i+1 < UserArgs.size()) {BinFile = UserArgs[i+1]; i++;}
                    
          std::string OutDir = ".";
          if(i+1 < UserArgs.size()) {OutDir = UserArgs[i+1]; i++;}
          
          bool bMakePackageList = true;
          if(i+1 < UserArgs.size() && UserArgs[i+1]=="no_lst") {bMakePackageList=false; i++;}
        
          Packager::goUnpack(BinFile,OutDir,bMakePackageList);
          exit(0); /* leaks memory too, but still nobody cares */
        }
      }
    }
  }

  /*============================================================================
  Viewport fun
  ============================================================================*/    
  void EditorApp::viewMessage(std::string Text) {
    int w = getTextWidth(Text);
    int h = getTextHeight(Text);
    drawText(Vector2f(m_ViewportPos.x + m_ViewportSize.x/2 - w/2,
                      m_ViewportPos.y + m_ViewportSize.y - h - 24),
                      Text,MAKE_COLOR(0,0,0,128),MAKE_COLOR(255,255,0,128));
  }
  
  void EditorApp::setViewRect(Vector2f Min,Vector2f Max) {
    float w = Max.x - Min.x;
    float h = w * ((float)getDispHeight() / (float)getDispWidth());
    float yc = (Max.y + Min.y)/2.0f;
    m_ViewMin = Vector2f( Min.x,yc-h/2 );
    m_ViewMax = Vector2f( Max.x,yc+h/2 );
  } 
  
  void EditorApp::setViewPoint(Vector2f Point) {
    float w = m_ViewMax.x - m_ViewMin.x;
    float h = m_ViewMax.y - m_ViewMin.y;
    m_ViewMin = Vector2f( Point.x - w/2,Point.y - h/2 );
    m_ViewMax = Vector2f( Point.x + w/2,Point.y + h/2 );    
  }
  
  void EditorApp::moveView(Vector2f Dir) {
    Vector2f Point = (m_ViewMin + m_ViewMax)*0.5f + Dir;
    float w = m_ViewMax.x - m_ViewMin.x;
    float h = m_ViewMax.y - m_ViewMin.y;
    m_ViewMin = Vector2f( Point.x - w/2,Point.y - h/2 );
    m_ViewMax = Vector2f( Point.x + w/2,Point.y + h/2 );    
  }
  
  void EditorApp::zoomView(float fFactor) {
    Vector2f Point = (m_ViewMin + m_ViewMax)*0.5f;
    float w = m_ViewMax.x - m_ViewMin.x;
    float h = m_ViewMax.y - m_ViewMin.y;
    m_ViewMin = Vector2f( Point.x - (w/2)*fFactor,Point.y - (h/2)*fFactor );
    m_ViewMax = Vector2f( Point.x + (w/2)*fFactor,Point.y + (h/2)*fFactor );    
  }
  
  void EditorApp::resetView(void) {
    setViewRect( Vector2f(m_pLevelSrc->getLeftLimit()-2,m_pLevelSrc->getBottomLimit()-2),
                 Vector2f(m_pLevelSrc->getRightLimit()+2,m_pLevelSrc->getTopLimit()+2));
  }
  
  void EditorApp::viewDrawLine(Vector2f P0,Vector2f P1,Color Col) {
    glBegin(GL_LINE_STRIP);
    glColor3f(GETF_RED(Col),GETF_GREEN(Col),GETF_BLUE(Col));
    viewVertex(P0);
    viewVertex(P1);
    glEnd();
  }
  
  void EditorApp::viewDrawCircle(Vector2f Cp,float Cr,float fBorder,Color BgColor,Color FgColor) {
    Vector2f V = Cp;
    Vector2f Cpt(m_ViewportPos.x + ((V.x-m_ViewMin.x)/(m_ViewMax.x-m_ViewMin.x))*m_ViewportSize.x,
                 m_ViewportPos.y + m_ViewportSize.y - ((V.y-m_ViewMin.y)/(m_ViewMax.y-m_ViewMin.y))*m_ViewportSize.y);
    if(Cr>0) {                 
      Vector2f CptX(m_ViewportPos.x + (((V.x + Cr)-m_ViewMin.x)/(m_ViewMax.x-m_ViewMin.x))*m_ViewportSize.x,
                  m_ViewportPos.y + m_ViewportSize.y - ((V.y-m_ViewMin.y)/(m_ViewMax.y-m_ViewMin.y))*m_ViewportSize.y);
      /* This could be done easier, but it's late and I refuse to use my brain :) */
      drawCircle(Cpt,fabs(CptX.x-Cpt.x),fBorder,BgColor,FgColor);
    }
    else {
      /* A negative Cr means they want to render with fixed radius */
      drawCircle(Cpt,-Cr,fBorder,BgColor,FgColor);
    }
  }
  
  void EditorApp::viewVertex(Vector2f V) {
    glVertex( m_ViewportPos.x + ((V.x-m_ViewMin.x)/(m_ViewMax.x-m_ViewMin.x))*m_ViewportSize.x,
              m_ViewportPos.y + m_ViewportSize.y - ((V.y-m_ViewMin.y)/(m_ViewMax.y-m_ViewMin.y))*m_ViewportSize.y );
  }
  
  void EditorApp::viewDrawText(Vector2f V,std::string Text,Color BgColor,Color FgColor) {
    Vector2f Cpt(m_ViewportPos.x + ((V.x-m_ViewMin.x)/(m_ViewMax.x-m_ViewMin.x))*m_ViewportSize.x,
                 m_ViewportPos.y + m_ViewportSize.y - ((V.y-m_ViewMin.y)/(m_ViewMax.y-m_ViewMin.y))*m_ViewportSize.y);
		float w = getTextWidth(Text);
		float h = getTextHeight(Text);
		drawText(Cpt - Vector2f(w/2,h/2),Text,BgColor,FgColor,true);
  }
  
  void EditorApp::viewDrawGrid(void) {      
    /* Draw one-meter level grid */
    for(float y=floor(m_pLevelSrc->getBottomLimit());y<=ceil(m_pLevelSrc->getTopLimit());y+=1.0f) {
      if(y>=m_ViewMax.y) break;
      if(y>=m_ViewMin.y) {
        viewDrawLine( Vector2f(m_pLevelSrc->getLeftLimit(),y),
                      Vector2f(m_pLevelSrc->getRightLimit(),y),MAKE_COLOR(120,120,120,255) );
      }
    }

    for(float x=floor(m_pLevelSrc->getLeftLimit());x<=ceil(m_pLevelSrc->getRightLimit());x+=1.0f) {
      if(x>=m_ViewMax.x) break;
      if(x>=m_ViewMin.x) {
        viewDrawLine( Vector2f(x,m_pLevelSrc->getTopLimit()),
                      Vector2f(x,m_pLevelSrc->getBottomLimit()),MAKE_COLOR(120,120,120,255) );
      }
    }
    
    if(m_CurState == ES_DEFAULT || m_CurState == ES_CREATING_NEW_BLOCK ||
       m_CurState == ES_CREATING_NEW_ENTITY) {
      /* Draw precission cursor */
      if(m_bCursorInViewport) {
        viewDrawLine( Vector2f( m_Cursor.x-1000,m_Cursor.y ),
                      Vector2f( m_Cursor.x+1000,m_Cursor.y ),
                      MAKE_COLOR(0,0,0,255) );
        viewDrawLine( Vector2f( m_Cursor.x,m_Cursor.y-1000 ),
                      Vector2f( m_Cursor.x,m_Cursor.y+1000 ),
                      MAKE_COLOR(0,0,0,255) );
      }
    }

    /* If we are in ES_MOVING_LIMITS, is we hovering? */
    Color LLc = MAKE_COLOR(0,0,0,255);
    Color RLc = MAKE_COLOR(0,0,0,255);
    Color TLc = MAKE_COLOR(0,0,0,255);
    Color BLc = MAKE_COLOR(0,0,0,255);
    bool bLL=false,bRL=false,bTL=false,bBL=false;
    
    if(m_CurState == ES_MOVING_LIMITS) {
      if(fabs(m_Cursor.x - m_pLevelSrc->getLeftLimit()) < 1.1) bLL=true;
      if(fabs(m_Cursor.y - m_pLevelSrc->getBottomLimit()) < 1.1) bBL=true;
      if(fabs(m_Cursor.x - m_pLevelSrc->getRightLimit()) < 1.1) bRL=true;
      if(fabs(m_Cursor.y - m_pLevelSrc->getTopLimit()) < 1.1) bTL=true;
      if(SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(1)) {
        if(bLL) {
          LLc = MAKE_COLOR(255,255,255,255); 
          m_pLevelSrc->setLimits( m_Cursor.x,m_pLevelSrc->getRightLimit(),
                                  m_pLevelSrc->getTopLimit(),m_pLevelSrc->getBottomLimit() );
        }
        if(bRL) {
          RLc = MAKE_COLOR(255,255,255,255); 
          m_pLevelSrc->setLimits( m_pLevelSrc->getLeftLimit(),m_Cursor.x,
                                  m_pLevelSrc->getTopLimit(),m_pLevelSrc->getBottomLimit() );
        }
        if(bTL) {
          TLc = MAKE_COLOR(255,255,255,255); 
          m_pLevelSrc->setLimits( m_pLevelSrc->getLeftLimit(),m_pLevelSrc->getRightLimit(),
                                  m_Cursor.y,m_pLevelSrc->getBottomLimit() );
        }
        if(bBL) {
          BLc = MAKE_COLOR(255,255,255,255); 
          m_pLevelSrc->setLimits( m_pLevelSrc->getLeftLimit(),m_pLevelSrc->getRightLimit(),
                                  m_pLevelSrc->getTopLimit(),m_Cursor.y );
        }
      }
      else {
        if(bLL) LLc = MAKE_COLOR(215,215,215,255); 
        if(bRL) RLc = MAKE_COLOR(215,215,215,255); 
        if(bTL) TLc = MAKE_COLOR(215,215,215,255); 
        if(bBL) BLc = MAKE_COLOR(215,215,215,255); 
      }
    }
    
    /* Draw blocks */
    for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
      Vector2f BP(pBlock->fPosX,pBlock->fPosY);
      
      if(m_SelMode != SM_BLOCKS || !_IsBlockSelected(pBlock)) {	
	      bool bSelectNext = false;
	      
	      if(m_SelMode == SM_EDGES && pBlock->Vertices[pBlock->Vertices.size()-1]->bSelected)
					bSelectNext = true;
  
        for(int j=0;j<pBlock->Vertices.size();j++) {
          Vector2f P=Vector2f(pBlock->Vertices[j]->fX,pBlock->Vertices[j]->fY) + BP;
          int jnext = j+1;
          if(jnext == pBlock->Vertices.size()) jnext=0;
          Vector2f PNext=Vector2f(pBlock->Vertices[jnext]->fX,pBlock->Vertices[jnext]->fY) + BP;          

          glBegin(GL_LINE_STRIP);          
          if(pBlock->bBackground) {
            if(pBlock->Vertices[j]->EdgeEffect != "")
              glColor3f(0,0,0.5);
            else
              glColor3f(0.2,0.9,0.2);
          }
          else if(pBlock->bWater) {
            if(pBlock->Vertices[j]->EdgeEffect != "")
              glColor3f(0,0,0.5);
            else
              glColor3f(0.2,0.2,0.9);
          }
          else {
            if(pBlock->Vertices[j]->EdgeEffect != "")
              glColor3f(0,0,1);
            else
              glColor3f(0.9,0.9,0.9);
          }
                    
          if(m_CurState == ES_MOVING_SELECTION && 
             (pBlock->Vertices[j]->bSelected || bSelectNext)) {
            P+=(m_Cursor - m_MStart);
                        
            if(m_SelMode == SM_EDGES && pBlock->Vertices[j]->bSelected) 
							bSelectNext = true;		
						else
							bSelectNext = false;				
          }
          else
						bSelectNext = false;          
				  
				  if(m_CurState == ES_MOVING_SELECTION && pBlock->Vertices[jnext]->bSelected) {
				    PNext+=(m_Cursor - m_MStart); 
				    bSelectNext = true;
				  }				  
				  else if(m_CurState == ES_MOVING_SELECTION && pBlock->Vertices[j]->bSelected &&
				     m_SelMode == SM_EDGES) {
				    PNext+=(m_Cursor - m_MStart);
				  }
                    
          viewVertex( P );
          viewVertex( PNext );
          glEnd();
        }
      }

      if(m_SelMode == SM_VERTICES || m_SelMode == SM_EDGES) {      
        /* Draw selected block vertices */
        for(int j=0;j<pBlock->Vertices.size();j++) {
          if(pBlock->Vertices[j]->bSelected) {
            Vector2f P=Vector2f(pBlock->Vertices[j]->fX,pBlock->Vertices[j]->fY) + BP;
            Vector2f PNext=Vector2f(pBlock->Vertices[0]->fX,pBlock->Vertices[0]->fY) + BP;
            if(j+1 < pBlock->Vertices.size())           
							PNext = Vector2f(pBlock->Vertices[j+1]->fX,pBlock->Vertices[j+1]->fY) + BP;
            
            if(m_CurState == ES_MOVING_SELECTION && pBlock->Vertices[j]->bSelected) {
              P+=(m_Cursor - m_MStart);
              PNext+=(m_Cursor - m_MStart);
            }
            
            if(m_SelMode == SM_EDGES) {
							glLineWidth(3);
							glBegin(GL_LINE_STRIP);        
							glColor4ub(255,0,0,255);
							viewVertex( P );
							viewVertex( PNext );
							glEnd();
							glLineWidth(1);
            }
            else {
							viewDrawCircle(P,-4,0,MAKE_COLOR(255,0,0,255),MAKE_COLOR(255,0,0,255));
						}
          }      
        }
      }
      else if(m_SelMode == SM_BLOCKS && _IsBlockSelected(pBlock)) {
        /* Draw block selected */
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);        
        glColor4ub(255,0,0,255);
        for(int j=0;j<pBlock->Vertices.size();j++) {
          Vector2f P=Vector2f(pBlock->Vertices[j]->fX,pBlock->Vertices[j]->fY) + BP;
          if(m_CurState == ES_MOVING_SELECTION)
            P+=(m_Cursor - m_MStart);          
          viewVertex( P );
        }
        glEnd();
        glLineWidth(1);
      }
    }
    
    
    /* Draw new block - if in ES_CREATING_NEW_BLOCK */
    if(m_CurState == ES_CREATING_NEW_BLOCK && m_NewBlockX.size()>0) {
      glBegin(GL_LINE_STRIP);
      glColor3f(1,0.5,0.2);
      for(int i=0;i<m_NewBlockX.size();i++) {
        viewVertex( Vector2f(m_NewBlockX[i],m_NewBlockY[i]) );
      }      
      viewVertex( m_Cursor );
      glEnd();
    }
    
    /* Draw entities */
    for(int i=0;i<m_pLevelSrc->Entities().size();i++) {
			_DrawEntitySymbol(m_pLevelSrc->Entities()[i]);
    }
    
    /* Draw selection box - if in ES_SELECTING */
    if(m_CurState == ES_SELECTING) {
      glBegin(GL_LINE_LOOP);
      glColor3f(0,1,0);
      viewVertex( m_Cursor );
      viewVertex( Vector2f(m_Cursor.x,m_SelPoint.y) );
      viewVertex( Vector2f(m_SelPoint.x,m_SelPoint.y) );
      viewVertex( Vector2f(m_SelPoint.x,m_Cursor.y) );      
      glEnd();
    }
      
    /* Draw level limits */
    viewDrawLine(Vector2f(m_pLevelSrc->getLeftLimit(),m_pLevelSrc->getBottomLimit()),
                 Vector2f(m_pLevelSrc->getRightLimit(),m_pLevelSrc->getBottomLimit()),BLc);
    viewDrawLine(Vector2f(m_pLevelSrc->getLeftLimit(),m_pLevelSrc->getTopLimit()),
                 Vector2f(m_pLevelSrc->getRightLimit(),m_pLevelSrc->getTopLimit()),TLc);
    viewDrawLine(Vector2f(m_pLevelSrc->getLeftLimit(),m_pLevelSrc->getBottomLimit()),
                 Vector2f(m_pLevelSrc->getLeftLimit(),m_pLevelSrc->getTopLimit()),LLc);
    viewDrawLine(Vector2f(m_pLevelSrc->getRightLimit(),m_pLevelSrc->getBottomLimit()),
                 Vector2f(m_pLevelSrc->getRightLimit(),m_pLevelSrc->getTopLimit()),RLc);                                      
  }
  
  /*============================================================================
  New level
  ============================================================================*/    
  void EditorApp::_NewLevel(void) {
    /* Do we already have a level? */
    if(m_pLevelSrc != NULL) {
      delete m_pLevelSrc;
    }
    
    /* Allocate new level */
    m_pLevelSrc = new LevelSrc;
    
    m_pLevelSrc->setScriptFile(std::string(""));
    m_pLevelSrc->setID(std::string("noname"));
    m_pLevelSrc->setFileName(std::string("Levels/noname.lvl"));
    
    m_pLevelSrc->getLevelInfo()->Name = "Unnamed Level";
    m_pLevelSrc->getLevelInfo()->Description = "";
    m_pLevelSrc->getLevelInfo()->Date = "";
    m_pLevelSrc->getLevelInfo()->Author = "";
    m_pLevelSrc->getLevelInfo()->Sky = "sky1";
    
    m_pLevelSrc->setLimits(-40,40,20,-20);
    
    /* Create PlayerStart entity in the middle of everything */
    _CreateEntityAtPos("PlayerStart",Vector2f(0,0));
    
    /* Set view */
    resetView();
    
    /* Set default state */
    setState(ES_DEFAULT);
  }

  /*============================================================================
  Main menu handler
  ============================================================================*/    
  void EditorMainMenu::itemSelected(int nItem) {
    switch(nItem) {
      case 0: /* Creating new level */
        m_pEditor->setState( ES_CREATING_NEW_LEVEL );
        break;
      case 1: /* Save level */
        m_pEditor->setState( ES_SAVING_LEVEL );
        break;
      case 2: /* Load level */
        m_pEditor->setState( ES_LOADING_LEVEL );
        break;
      case 3: /* Play level */
        m_pEditor->setState( ES_PLAYING_LEVEL );
        break;
      case 5: /* Move limits */
        m_pEditor->setState( ES_MOVING_LIMITS );
        break;
      case 6: /* Create new block */
        m_pEditor->setState( ES_CREATING_NEW_BLOCK );
        break;
      case 7: /* Edit selected entity */
				m_pEditor->setState( ES_EDITING_SELECTED_ENTITY );
				break;
			case 8: /* Edit level properties */
			  m_pEditor->setState( ES_EDITING_LEVEL_PROPS );
			  break;
			case 9: /* Edit edge effect */
			  m_pEditor->setState( ES_EDITING_EDGE_EFFECT );
			  break;
			case 10: /* Smooth edges */
			  m_pEditor->setState( ES_SMOOTHING_EDGES );
			  break;
			case 11: /* Delete selection */
			  m_pEditor->setState( ES_DELETING_SELECTION );
			  break;
			case 12: /* Toggle block background */
			  m_pEditor->setState( ES_TOGGLE_BG_BLOCK );
			  break;
			//case 13: /* Toggle block water */
			//  m_pEditor->setState( ES_TOGGLE_WATER_BLOCK );
			//  break;
			case 13: /* Copy entity */
			  m_pEditor->setState( ES_COPYING_ENTITY );
			  break;
      case 15: /* Quit */
        m_pEditor->quit();
        break;
    }
  }  
  
  /*============================================================================
  Entity menu handler
  ============================================================================*/    
  void EditorEntityMenu::itemSelected(int nItem) {
		/* Go into the entity placing state */
		m_pEditor->setState( ES_CREATING_NEW_ENTITY,nItem );
  }  
  
  /*============================================================================
  Draw menu
  ============================================================================*/    
  void EditorMenu::drawMenu(EditorApp *pEditor,const Vector2f &Pos) {
    int nX,nY,w;
    SDL_GetMouseState(&nX,&nY);
    Vector2f CPos = Pos;
    
    /* Get max item length */
    w=0;
    for(int i=0;i<m_Items.size();i++)
      if(m_Items[i].length() * 8 > w) w=m_Items[i].length() * 8;
   
    /* Start drawing */
    for(int i=0;i<m_Items.size();i++) {
      if(nX >= CPos.x && nY >= CPos.y && nX < CPos.x+w && nY < CPos.y+12) {
        pEditor->drawBox( CPos,CPos+Vector2f(w,12),0,MAKE_COLOR(255,0,0,255),0 );  

        /* Selected? */
        if( pEditor->isLeftButtonClicked() ) {
          itemSelected( i );
        }
      }
      pEditor->drawText( CPos,m_Items[i],0,MAKE_COLOR(255,255,0,255) );
      CPos.y += 12;      
    }
  }
  
  /*============================================================================
  Changing states
  ============================================================================*/    
  void EditorApp::setState(EditorState s,int nOptParam) {
    LevelEntity *pEntity;
    
    m_CurState = s;
    
    switch(s) {
      case ES_CREATING_NEW_BLOCK:
        while(!m_NewBlockX.empty()) {
          m_NewBlockX.erase(m_NewBlockX.begin());
          m_NewBlockY.erase(m_NewBlockY.begin());
        }        
        break;
      case ES_SAVING_LEVEL:
        /* Save level! */
        m_Log.msg("Saving level in '%s'...",m_pLevelSrc->getFileName().c_str());
        m_pLevelSrc->saveXML();
        setState(ES_DEFAULT);
        break;
      case ES_SELECTING:
        m_SelPoint = m_Cursor;
        break;
      case ES_MOVING_SELECTION:
        m_MStart = m_Cursor;
        break;
      case ES_CREATING_NEW_ENTITY:
        if(nOptParam >= 0 && nOptParam<m_EntityTable.getTypes().size())
				  m_NewEntityID = m_EntityTable.getTypes()[nOptParam]->ID;
				else
				  setState(ES_DEFAULT);
				break;
      case ES_DELETING_SELECTION:
        if(m_SelMode == SM_BLOCKS) {
          _DeleteSelectedBlocks();
          setState(ES_DEFAULT);
        }
        else if(m_SelMode == SM_VERTICES) {
          _DeleteSelectedVertices();
          setState(ES_DEFAULT);
        }
        else if(m_SelMode == SM_EDGES) {
          _DeleteSelectedEdges();
          setState(ES_DEFAULT);
        }
        else if(m_SelMode == SM_ENTITIES) {
          _DeleteSelectedEntities();
          setState(ES_DEFAULT);
        }
        else m_Log.msg("Please select a mode where you can delete something.");
        break;
			case ES_EDITING_SELECTED_ENTITY:
				/* Get selected entity */
				pEntity = _GetSelectedEntity();
				if(pEntity!=NULL) {				
					/* Open parameter editor */
					PETEntry Entries[100];
					int nNumEntries = 0;																																

          Entries[0].Name = "ID";
          Entries[0].Value = pEntity->ID;
          Entries[0].OrigValue = pEntity->ID;
          nNumEntries = 1;
															
					for(int i=0;i<pEntity->Params.size();i++) {
						Entries[nNumEntries].Name = pEntity->Params[i]->Name;
						Entries[nNumEntries].Value = pEntity->Params[i]->Value;
						Entries[nNumEntries].OrigValue = pEntity->Params[i]->Value;
						nNumEntries++;
					}
					
					ParamEditTool PET;
					PET.setTable(Entries,nNumEntries);

					Vector2f A=Vector2f(17,17);
					Vector2f B=Vector2f(getDispWidth()-66,getDispHeight()-27);
					//glScissor(A.x,getDispHeight() - B.y-1,B.x-A.x+1,B.y-A.y+1);
          scissorGraphics(A.x,A.y,(B.x - A.x),(B.y - A.y));
					glClearColor(0.1,0.1,0.3,0);
					glEnable(GL_SCISSOR_TEST);
					PET.setDims(A,B);
					
					if(PET.run(this)) {
						m_Log.msg("Parameters changed for '%s'.",pEntity->ID.c_str());
						
						/* Mkay. */
						pEntity->ID = Entries[0].Value;
						for(int i=0;i<pEntity->Params.size();i++) {
						  pEntity->Params[i]->Value = Entries[i+1].Value;
						}
					}

					glDisable(GL_SCISSOR_TEST);
				}
								
				setState(ES_DEFAULT);
				break;			
					
			case ES_EDITING_LEVEL_PROPS:
				/* Open parameter editor */
				{
				  PETEntry Entries[100];
				  int nNumEntries = 8;																																

          Entries[0].Name = "Filename";
          Entries[0].OrigValue = Entries[0].Value = m_pLevelSrc->getFileName();

          Entries[1].Name = "ID";
          Entries[1].OrigValue = Entries[1].Value = m_pLevelSrc->getID();

          Entries[2].Name = "Script";
          Entries[2].OrigValue = Entries[2].Value = m_pLevelSrc->getScriptFile();

          Entries[3].Name = "Name";
          Entries[3].OrigValue = Entries[3].Value = m_pLevelSrc->getLevelInfo()->Name;
          
          Entries[4].Name = "Date";
          Entries[4].OrigValue = Entries[4].Value = m_pLevelSrc->getLevelInfo()->Date;

          Entries[5].Name = "Author";
          Entries[5].OrigValue = Entries[5].Value = m_pLevelSrc->getLevelInfo()->Author;

          Entries[6].Name = "Description";
          Entries[6].OrigValue = Entries[6].Value = m_pLevelSrc->getLevelInfo()->Description;

          Entries[7].Name = "Sky";
          Entries[7].OrigValue = Entries[7].Value = m_pLevelSrc->getLevelInfo()->Sky;
  																		
				  ParamEditTool PET;
				  PET.setTable(Entries,nNumEntries);

				  Vector2f A=Vector2f(17,17);
				  Vector2f B=Vector2f(getDispWidth()-66,getDispHeight()-27);
				  //glScissor(A.x,getDispHeight() - B.y-1,B.x-A.x+1,B.y-A.y+1);
          scissorGraphics(A.x,A.y,(B.x - A.x),(B.y - A.y));
				  glClearColor(0.1,0.1,0.3,0);
				  glEnable(GL_SCISSOR_TEST);
				  PET.setDims(A,B);
  				
				  if(PET.run(this)) {
					  m_Log.msg("Level parameters changed.");
  					
            m_pLevelSrc->setFileName( Entries[0].Value );
            
            /* Make sure invalid chars isn't saved in ID */
            std::string ID = Entries[1].Value;

            int i=0;
            while(1) {
              if(i >= ID.length()) break;
              
              if((ID[i] >= 'a' && ID[i] <= 'z') ||
                (ID[i] >= 'A' && ID[i] <= 'Z') ||
                (ID[i] >= '0' && ID[i] <= '9') || ID[i]=='_') {
                /* This is ok */
                i++;
              }
              else {
                /* Not ok */
                ID.erase(ID.begin() + i);
              }            
            }
            
            if(ID == "") ID = "noname";
                        
            m_pLevelSrc->setID(ID);
            
            m_pLevelSrc->setScriptFile( Entries[2].Value );
            m_pLevelSrc->getLevelInfo()->Name = Entries[3].Value;
            m_pLevelSrc->getLevelInfo()->Date = Entries[4].Value;
            m_pLevelSrc->getLevelInfo()->Author = Entries[5].Value;
            m_pLevelSrc->getLevelInfo()->Description = Entries[6].Value;          
            m_pLevelSrc->getLevelInfo()->Sky = Entries[7].Value;          
				  }

				  glDisable(GL_SCISSOR_TEST);
				  setState(ES_DEFAULT);
				}
			  break;
			  
			case ES_EDITING_EDGE_EFFECT:
				/* Get current value */
				if(m_SelMode == SM_EDGES) {
				  int nNum=0;
				  std::string Val = _GetCommonEdgeEffect(&nNum);
				  if(nNum>0) {
				    PETEntry Entries[100];
				    int nNumEntries = 1;																																

            Entries[0].Name = "Edge effect";
            Entries[0].OrigValue = Entries[0].Value = Val;
				    
				    ParamEditTool PET;
				    PET.setTable(Entries,nNumEntries);

				    Vector2f A=Vector2f(17,17);
				    Vector2f B=Vector2f(getDispWidth()-66,getDispHeight()-27);
				    //glScissor(A.x,getDispHeight() - B.y-1,B.x-A.x+1,B.y-A.y+1);
            scissorGraphics(A.x,A.y,(B.x - A.x),(B.y - A.y));
				    
				    glClearColor(0.1,0.1,0.3,0);
				    glEnable(GL_SCISSOR_TEST);
				    PET.setDims(A,B);
    				
				    if(PET.run(this)) {
					    m_Log.msg("Edge effect changed.");
              _ApplyEdgeEffect(Entries[0].Value);    					
				    }

				    glDisable(GL_SCISSOR_TEST);
				  }
				  else
				    m_Log.msg("No edge selected!");
				}
				else
				  m_Log.msg("Only possible in edge-selection mode!");				  
  	    setState(ES_DEFAULT);				    
			  break;			
			  
			case ES_CREATING_NEW_LEVEL:
        _NewLevel();
        setState(ES_DEFAULT);			  
			  break;
			  
			case ES_SMOOTHING_EDGES: 
			  _SmoothSelectedEdges();
			  m_Log.msg("Captain! Your edges are now much smoother!");
        setState(ES_DEFAULT);			  
			  break;

			case ES_LOADING_LEVEL:{		 
			  /* Find out what to load */
			  /* TODO: A fancy file browser would be nice :P */
			  
			  InputBox IB;
			  IB.setInit("Write name of level file to load...","");

				Vector2f A=Vector2f(17,17);
				Vector2f B=Vector2f(getDispWidth()-66,getDispHeight()-27);
				//glScissor(A.x,getDispHeight() - B.y-1,B.x-A.x+1,B.y-A.y+1);
				            scissorGraphics(A.x,A.y,(B.x - A.x),(B.y - A.y));

				glClearColor(0.1,0.1,0.3,0);
				glEnable(GL_SCISSOR_TEST);
				IB.setDims(A,B);
			  
			  if(IB.run(this)) {
			    std::string Path = std::string("Levels/") + IB.getInput();
			  
          m_Log.msg("Loading level from '%s'...",Path.c_str());
          
          m_pLevelSrc->setFileName( Path );
          m_pLevelSrc->loadXML();
			  }

		    glDisable(GL_SCISSOR_TEST);
        setState(ES_DEFAULT);			  
			  			  
			  break;
			}
			case ES_PLAYING_LEVEL: {
        m_Log.msg("Saving level in '%s'...",m_pLevelSrc->getFileName().c_str());
        m_pLevelSrc->saveXML();
        m_Log.msg("Playing it.");
        
        char cBuf[256];
        sprintf(cBuf,"%s -win -level \"%s\"",XMOTOCOMMAND,m_pLevelSrc->getID().c_str());
        system(cBuf);      
        setState(ES_DEFAULT);			          
			  break;
			}
			case ES_TOGGLE_BG_BLOCK: {
			  _ToggleSelectedBlockBackground();
			  setState(ES_DEFAULT);
			  break;
			}
			case ES_TOGGLE_WATER_BLOCK: {
			  _ToggleSelectedBlockWater();
			  setState(ES_DEFAULT);
			  break;
			}
			case ES_COPYING_ENTITY: {
				/* Get selected entity */
				pEntity = _GetSelectedEntity();
				if(pEntity!=NULL) {				
				  m_pEntityToCopy = pEntity;
				}
				else {
				  m_Log.msg("Please select the entity to copy first...");
			    setState(ES_DEFAULT);
			  }
			  break;
			}
    }
  }
  
  /*============================================================================
  Calculate average value of float array
  ============================================================================*/    
  float EditorApp::_GetAvg(std::vector<float> &x) {
    float v = 0.0f;
    if(x.empty()) return 0.0f;
    for(int i=0;i<x.size();i++) v+=x[i];
    return v/(float)x.size();
  }
  
  /*============================================================================
  Default name creators
  ============================================================================*/    
  std::string EditorApp::_CreateBlockName(void) {
    /* Probe for free name */
    for(int i=0;i<100000;i++) {
      char cBuf[256];
      sprintf(cBuf,"Block%d",i);
      if(m_pLevelSrc->getBlockByID( cBuf ) == NULL) return cBuf;      
    }
    throw Exception("Too many blocks");    
  }
  
  std::string EditorApp::_CreateZoneName(void) {
    /* Probe for free name */
    for(int i=0;i<100000;i++) {
      char cBuf[256];
      sprintf(cBuf,"Zone%d",i);
      if(m_pLevelSrc->getZoneByID( cBuf ) == NULL) return cBuf;      
    }
    throw Exception("Too many zones");    
  }
  
  /*============================================================================
  Select by selection box
  ============================================================================*/    
  void EditorApp::_SelectByBox(void) {
    /* Get min/max */    
    Vector2f SMin( m_Cursor.x<m_SelPoint.x ? m_Cursor.x:m_SelPoint.x,
                   m_Cursor.y<m_SelPoint.y ? m_Cursor.y:m_SelPoint.y );
    Vector2f SMax( m_Cursor.x>m_SelPoint.x ? m_Cursor.x:m_SelPoint.x,
                   m_Cursor.y>m_SelPoint.y ? m_Cursor.y:m_SelPoint.y );

    if(m_nGridSnap) {
			SMin = SMin - Vector2f((1.0f/m_nGridSnap) / 4.0f,(1.0f/m_nGridSnap) / 4.0f);
			SMax = SMax + Vector2f((1.0f/m_nGridSnap) / 4.0f,(1.0f/m_nGridSnap) / 4.0f);
    }		
    
    /* Selecting entities? */
    if(m_SelMode == SM_ENTITIES) {
			/* Yup */
			for(int i=0;i<m_pLevelSrc->Entities().size();i++) {
				LevelEntity *pEntity = m_pLevelSrc->Entities()[i];
				EditorEntityType *pType = m_EntityTable.getTypeByID(pEntity->TypeID);				
				if(pType == NULL) continue;
				
				if(circleTouchAABB2f(Vector2f(pEntity->fPosX,pEntity->fPosY),pType->fSize,
				                     SMin,SMax)) {
					pEntity->bSelected = true;
				}
				else {
					pEntity->bSelected = false;
				}
			}
    }
    else {
			/* No, something else */            
			/* Select all stuff within this region */
			/* -- first vertices... */
			for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
				LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
				for(int j=0;j<pBlock->Vertices.size();j++) {
					LevelBlockVertex *pVertex = pBlock->Vertices[j];
					if(pointTouchAABB2f(Vector2f(pVertex->fX,pVertex->fY) + Vector2f(pBlock->fPosX,pBlock->fPosY),
															SMin,SMax)) {
						/* Mkay! */
						pVertex->bSelected = true;          
					}
					else {
						/* No selection here, thank you */
						pVertex->bSelected = false;
					}
				}
			}
	    
			/* If in edge or block selection, then look for edges too... */
			if(m_SelMode == SM_BLOCKS || m_SelMode == SM_EDGES) {
				for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
					LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
					for(int j=0;j<pBlock->Vertices.size();j++) {
						/* Define edge */
						LevelBlockVertex *pVertex = pBlock->Vertices[j];
						LevelBlockVertex *pNextVertex = pBlock->Vertices[0];
						if(j+1 < pBlock->Vertices.size())
							pNextVertex = pBlock->Vertices[j+1];
							
						/* Does it touch the box? */
						if(lineTouchAABB2f(Vector2f(pVertex->fX,pVertex->fY) + Vector2f(pBlock->fPosX,pBlock->fPosY),
															Vector2f(pNextVertex->fX,pNextVertex->fY) + Vector2f(pBlock->fPosX,pBlock->fPosY),
															SMin,SMax)) {
							/* Select vertex (edge) or entire block? */
							if(m_SelMode == SM_EDGES) 
								pVertex->bSelected = true;
							else {
								_SelectBlock(pBlock);
								break; /* next block, go go */
							}
						}					                   
					}
				}
			}
		}
  }

  /*============================================================================
  Anything at point?
  ============================================================================*/    
  ClickTarget EditorApp::_AnythingAtPoint(Vector2f P,bool bSelectAtWill) {
    /* Get min/max */    
    Vector2f SMin = P;
    Vector2f SMax = P;
    
    if(m_nGridSnap) {
			SMin = SMin - Vector2f((1.0f/m_nGridSnap) / 4.0f,(1.0f/m_nGridSnap) / 4.0f);
			SMax = SMax + Vector2f((1.0f/m_nGridSnap) / 4.0f,(1.0f/m_nGridSnap) / 4.0f);
    }		
    else {
			SMin = SMin - Vector2f(0.1f,0.1f);
			SMax = SMax + Vector2f(0.1f,0.1f);
    }
    
		/* Selecting entities? */
		if(m_SelMode == SM_ENTITIES) {
			/* Yes */
			for(int i=0;i<m_pLevelSrc->Entities().size();i++) {
				LevelEntity *pEntity = m_pLevelSrc->Entities()[i];
				EditorEntityType *pType = m_EntityTable.getTypeByID(pEntity->TypeID);				
				if(pType == NULL) continue;
				
				if(circleTouchAABB2f(Vector2f(pEntity->fPosX,pEntity->fPosY),pType->fSize,
				                     SMin,SMax)) {
					/* We found something. Is it already selected? */
					if(pEntity->bSelected) return CT_SELECTION;
					
					if(bSelectAtWill) {
						_ClearSelection();
						pEntity->bSelected = true;
						return CT_SELECTION;
					}
					return CT_SINGLE_OBJECT;
				}
			}
		}
		else { 
      /* No */ 
			for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
				LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
				for(int j=0;j<pBlock->Vertices.size();j++) {
					LevelBlockVertex *pVertex = pBlock->Vertices[j];
	        
					/* Point? */
					if(pointTouchAABB2f(Vector2f(pVertex->fX,pVertex->fY) + Vector2f(pBlock->fPosX,pBlock->fPosY),
															SMin,SMax)) {
						/* We have a point here */
						if(pVertex->bSelected) return CT_SELECTION;
	          
						if(bSelectAtWill) {
							_ClearSelection();            
							pVertex->bSelected = true;
							return CT_SELECTION;
						}
						return CT_SINGLE_OBJECT;                            
					}
	        
					/* Edge? */
					if(m_SelMode == SM_BLOCKS || m_SelMode == SM_EDGES) {
						LevelBlockVertex *pNextVertex = pBlock->Vertices[0];
						if(j+1 < pBlock->Vertices.size()) pNextVertex = pBlock->Vertices[j+1];
						if(lineTouchAABB2f(Vector2f(pVertex->fX,pVertex->fY) + Vector2f(pBlock->fPosX,pBlock->fPosY),
															Vector2f(pNextVertex->fX,pNextVertex->fY) + Vector2f(pBlock->fPosX,pBlock->fPosY),
															SMin,SMax)) {
							if(pVertex->bSelected) return CT_SELECTION;
		          
							if(bSelectAtWill) {
								_ClearSelection();
								pVertex->bSelected = true;
								return CT_SELECTION;
							}
							return CT_SINGLE_OBJECT;
						}
					}
				}
      }
    }
    
    /* Nope */
    return CT_NOTHING;
  }

  /*============================================================================
  Deselect everything
  ============================================================================*/    
  void EditorApp::_ClearSelection(void) {
		/* Deselect all block-related */
    for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
      for(int j=0;j<pBlock->Vertices.size();j++) {
        LevelBlockVertex *pVertex = pBlock->Vertices[j];
        pVertex->bSelected = false;
      }
    }    
    
    /* Deselect all entities */
    for(int i=0;i<m_pLevelSrc->Entities().size();i++) {
			LevelEntity *pEntity = m_pLevelSrc->Entities()[i];
			pEntity->bSelected = false;
		}
  }
  
  /*============================================================================
  Apply movement to selection
  ============================================================================*/    
  void EditorApp::_FinishMovement(Vector2f Dir) {
		/* Moving entities? */
		if(m_SelMode == SM_ENTITIES) {
			for(int i=0;i<m_pLevelSrc->Entities().size();i++) {
				LevelEntity *pEntity = m_pLevelSrc->Entities()[i];
				if(pEntity->bSelected) {
					pEntity->fPosX += (m_Cursor.x - m_MStart.x);
					pEntity->fPosY += (m_Cursor.y - m_MStart.y);
				}
			}			
		}
		else {
			for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
				bool bUpdateBlockCoords = false;
				Vector2f Center(0,0);
	      
				LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
				for(int j=0;j<pBlock->Vertices.size();j++) {
					LevelBlockVertex *pVertex = pBlock->Vertices[j];
					if(m_SelMode == SM_VERTICES) {
						if(pVertex->bSelected) {
							pVertex->fX += Dir.x;
							pVertex->fY += Dir.y;
						}
					}
					if(m_SelMode == SM_EDGES) {
						if(pVertex->bSelected) {
							pVertex->fX += Dir.x;
							pVertex->fY += Dir.y;
						}
						else {
							LevelBlockVertex *pPrev;
							if(j==0) pPrev = pBlock->Vertices[pBlock->Vertices.size()-1];
							else pPrev = pBlock->Vertices[j-1];
							
							if(pPrev->bSelected) {
								pVertex->fX += Dir.x;
								pVertex->fY += Dir.y;
							}
						}
					}
					else if(m_SelMode == SM_BLOCKS) {
						if(_IsBlockSelected(pBlock)) {
							pVertex->fX += Dir.x;
							pVertex->fY += Dir.y;
						}
					}
	        
					Center.x += pVertex->fX;
					Center.y += pVertex->fY;
	        
					/* TODO: Update texture coordinates and stuff! */
	                
					bUpdateBlockCoords = true; /* remember to recalculate block center */
				}
	      
				if(bUpdateBlockCoords && !pBlock->Vertices.empty()) {
					Center.x /= (float)pBlock->Vertices.size();
					Center.y /= (float)pBlock->Vertices.size();
					for(int j=0;j<pBlock->Vertices.size();j++) {
						/* TODO: TEXTURE COORDS!!! :-O */
						LevelBlockVertex *pVertex = pBlock->Vertices[j];
						pVertex->fX = (pVertex->fX + pBlock->fPosX) - Center.x;
						pVertex->fY = (pVertex->fY + pBlock->fPosY) - Center.y;                    
					}
	        
					/* Upd to new center */
					pBlock->fPosX = Center.x;
					pBlock->fPosY = Center.y;
				}
      }
    }    
  }
  
  /*============================================================================
  Simple selector (grid-snap settings, selection settings, etc)
  ============================================================================*/    
  void EditorSimpleSelector::drawSimpleSelector(EditorApp *pEditor,const Vector2f &Pos) {
    Vector2f c = Pos;
    pEditor->drawText( c,m_Name,0,MAKE_COLOR(255,255,255,255) );
    c.x+=m_Name.length()*8+8;
    for(int i=0;i<m_Items.size();i++) {
      /* Is cursor above item? */
      Vector2f c2=c+Vector2f( m_Items[i].length()*8,12 );
      int nX,nY;
      SDL_GetMouseState(&nX,&nY);
      bool bHover = false;
      
      if(nX>=c.x && nY>=c.y && nX<c2.x && nY<c2.y)
        bHover = true;
      
      /* If this item is selected, draw background */
      if(i == m_nSelIdx) {
        pEditor->drawBox( c,c2,0,MAKE_COLOR(255,0,0,255),0 );
      }
      
      /* Draw */
      if(bHover) {
        if(i == m_nSelIdx)
          pEditor->drawText( c,m_Items[i],0,MAKE_COLOR(255,255,0,255) );
        else
          pEditor->drawText( c,m_Items[i],0,MAKE_COLOR(255,255,255,255) );
          
        if(pEditor->isLeftButtonClicked()) {
          m_nSelIdx=i;
        }
      }
      else
        pEditor->drawText( c,m_Items[i],0,MAKE_COLOR(128,128,128,255) );
                
      /* Move on */
      c.x += m_Items[i].length()*8+8;
    }
  }
  
  float EditorSimpleSelector::getWidth(void) {
    float w=0.0f;
    for(int i=0;i<m_Items.size();i++) w+=m_Items[i].length() * 8 + 8;
    w+=m_Name.length() * 8 + 8;
    return w;
  }
  
  /*============================================================================
  Check if parts of the block are selected
  ============================================================================*/    
  bool EditorApp::_IsBlockSelected(LevelBlock *pBlock) {
    for(int i=0;i<pBlock->Vertices.size();i++)
      if(pBlock->Vertices[i]->bSelected) return true;
    return false;
  }
  
  /*============================================================================
  Select all vertices of block
  ============================================================================*/    
  void EditorApp::_SelectBlock(LevelBlock *pBlock) {
    for(int i=0;i<pBlock->Vertices.size();i++)
			pBlock->Vertices[i]->bSelected = true;
  }

  /*============================================================================
  If all blocks in list share the same texture, return an ref to it
  ============================================================================*/    
  Texture *EditorApp::_GetCommonTexture(std::vector<LevelBlock *> &Blocks) {
    std::string CommonTextureID = "";
    
    for(int i=0;i<Blocks.size();i++) {
      if(CommonTextureID!="" && Blocks[i]->Texture!=CommonTextureID) {
        return NULL; /* no common one, shit happens */  
      }      
      CommonTextureID = Blocks[i]->Texture;
    }  

    Sprite* v_sprite = m_theme.getSprite(SPRITE_TYPE_TEXTURE, CommonTextureID);
    if(v_sprite == NULL) {
      return NULL;
    }

    return v_sprite->getTexture();   
  }
  
  /*============================================================================
  Create an instance of the given entity-type at location
  ============================================================================*/    
	void EditorApp::_CreateEntityAtPos(std::string TypeID,Vector2f Pos) {
		LevelEntity *pEntity = m_pLevelSrc->createEntity(TypeID,Pos.x,Pos.y);
		if(pEntity == NULL) return; /* TODO: visible warning */
		
		/* Look up type in table, and add default parameters */
	  EditorEntityType *pType = m_EntityTable.getTypeByID(TypeID);
	  if(pType != NULL) {
	    pEntity->fSize = pType->fPSize;
	  
			for(int i=0;i<pType->Params.size();i++) {
				LevelEntityParam *pParam = new LevelEntityParam;
				pParam->Name = pType->Params[i]->Name;
				pParam->Value = pType->Params[i]->DefaultValue;
				pEntity->Params.push_back( pParam );
			}
	  }
	}
	
  /*============================================================================
  Entity copying
  ============================================================================*/    
  void EditorApp::_CopyEntityAtPos(LevelEntity *pToCopy,Vector2f Pos) {
    if(pToCopy != NULL) {
		  LevelEntity *pEntity = m_pLevelSrc->createEntity(pToCopy->TypeID,Pos.x,Pos.y);
		  if(pEntity == NULL) return; /* TODO: visible warning */
		  
		  pEntity->bSelected = false;
		  pEntity->fSize = pToCopy->fSize;
		  
		  m_Log.msg("Entity '%s' copied into '%s'...",pToCopy->ID.c_str(),pEntity->ID.c_str());
		 
		  for(int i=0;i<pToCopy->Params.size();i++) {
		    LevelEntityParam *pParam = new LevelEntityParam;
		    pParam->Name = pToCopy->Params[i]->Name;
		    pParam->Value = pToCopy->Params[i]->Value;
		    pEntity->Params.push_back( pParam );
		  }
		}
  }
	
  /*============================================================================
  Get selected entity
  ============================================================================*/
	LevelEntity *EditorApp::_GetSelectedEntity(void) {
		LevelEntity *pRet = NULL;
		for(int i=0;i<m_pLevelSrc->Entities().size();i++) {
			if(m_pLevelSrc->Entities()[i]->bSelected) {
			  if(pRet == NULL) pRet = m_pLevelSrc->Entities()[i];
			  else {
					m_Log.msg("More than one entity is selected!");
					return NULL;
			  }
			}
		}		
		if(pRet == NULL) m_Log.msg("No entity is selected!");
		return pRet;
	}
	
  /*============================================================================
  If all selected edges (vertices) have the same edge effect, return it
  ============================================================================*/
  std::string EditorApp::_GetCommonEdgeEffect(int *pnNumSel) {
    std::string Ret = "";
    bool bEffect = false;
    for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      for(int j=0;j<m_pLevelSrc->Blocks()[i]->Vertices.size();j++) {
        LevelBlockVertex *pVertex = m_pLevelSrc->Blocks()[i]->Vertices[j];
        
        if(pVertex->bSelected) {
          if(pnNumSel!=NULL) *pnNumSel = *pnNumSel+1;
          if(!bEffect) {
            Ret = pVertex->EdgeEffect;
            bEffect = true;
          }
          else {
            if(pVertex->EdgeEffect != Ret) return "";
          }
        }
      }
    }
    return Ret;
  }

  /*============================================================================
  Apply given edge effect string to selected edges
  ============================================================================*/
  void EditorApp::_ApplyEdgeEffect(std::string Effect) {
    for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      for(int j=0;j<m_pLevelSrc->Blocks()[i]->Vertices.size();j++) {
        LevelBlockVertex *pVertex = m_pLevelSrc->Blocks()[i]->Vertices[j];
        if(pVertex->bSelected)
          pVertex->EdgeEffect = Effect;
      }
    }
  }

  /*============================================================================
  Draw entity symbol
  ============================================================================*/
	void EditorApp::_DrawEntitySymbol(LevelEntity *pEntity) {
		/* Fetch type and position */
		EditorEntityType *pType = m_EntityTable.getTypeByID(pEntity->TypeID);
		Vector2f Pos = Vector2f(pEntity->fPosX,pEntity->fPosY);
		
		if(pType == NULL) return; /* TODO: error */
		
		/* Selected and moving? */
		Vector2f Adj(0,0);
		if(pEntity->bSelected && m_CurState==ES_MOVING_SELECTION && m_SelMode==SM_ENTITIES)
			Adj = m_Cursor - m_MStart;
		
		/* Draw elements */
		for(int i=0;i<pType->DrawProc.size();i++) {
			ETDraw *pDraw = pType->DrawProc[i];
			switch(pDraw->Type) {
				case ET_CIRCLE:
					if(pEntity->bSelected)
						viewDrawCircle(Pos+Adj,pDraw->fRadius,pDraw->fBorder+2,pDraw->BColor,MAKE_COLOR(255,0,0,255));
					else
						viewDrawCircle(Pos+Adj,pDraw->fRadius,pDraw->fBorder,pDraw->BColor,pDraw->FColor);
					break;
				case ET_TEXT:
					if(pEntity->bSelected)
						viewDrawText(Pos+Adj,pDraw->Caption,pDraw->BColor,MAKE_COLOR(255,0,0,255));
					else
						viewDrawText(Pos+Adj,pDraw->Caption,pDraw->BColor,pDraw->FColor);
					break;
				case ET_RECT:
					//viewDra
					break;				
			}
		}
	}
  
  /*============================================================================
  Drawing of texture browsing box
  ============================================================================*/
  void EditorApp::_DrawTextureBrowser(void) {
    if(m_SelMode == SM_BLOCKS) {
      /* Create list of selected blocks */
      std::vector<LevelBlock *> Selection;
      
      for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
        LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
        if(_IsBlockSelected(pBlock)) Selection.push_back(pBlock);
      }
      
      /* Show ID */
      if(Selection.size() == 1) {
        drawText(m_TextureBrowserPos+Vector2f(1,1),"Block sel.:");
        drawText(m_TextureBrowserPos+Vector2f(1,13),Selection[0]->ID);        
      }
      else if(Selection.size() > 1) {
        drawText(m_TextureBrowserPos+Vector2f(1,1),"Block sel.:");
        char cBuf[256];
        sprintf(cBuf,"(%d selected)",Selection.size());
        drawText(m_TextureBrowserPos+Vector2f(1,13),cBuf);        
      }
      
      /* Draw texture */
      if(Selection.size() > 0) {              
        Texture *pTexture = _GetCommonTexture(Selection);
        if(pTexture != NULL) {
          /* Find out what size it should have... */
          drawImage(m_TextureBrowserPos+Vector2f(9,25),m_TextureBrowserPos+m_TextureBrowserSize
                    -Vector2f(14,8),pTexture);
          
          Color c = MAKE_COLOR(255,255,0,255);
          if(m_bTextureBrowserMouseHover) {
            c = MAKE_COLOR(0,255,0,255);
            drawBox(m_TextureBrowserPos+Vector2f(9,25),m_TextureBrowserPos+m_TextureBrowserSize
                    -Vector2f(14,8),1,0,c);
          }                    
                    
          drawText(m_TextureBrowserPos+Vector2f(9,25),pTexture->Name,0,c);
        } 
        else {
          drawText(m_TextureBrowserPos+Vector2f(1,25),"(Misc. textures)");     
          
          if(m_bTextureBrowserMouseHover) {
            drawText(m_TextureBrowserPos+Vector2f(9,37),"Select another",0,MAKE_COLOR(0,255,0,255));
          }
        }
          
        return;
      }      
    }

    /* Not in block selection mode */
    drawText(m_TextureBrowserPos+Vector2f(1,1),"No blocks sel.");
  }

  /*============================================================================
  All textures with selected vertices will get this texture
  ============================================================================*/
  void EditorApp::_AssignTextureToSelection(Texture *pTexture) {
    for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
      if(_IsBlockSelected(pBlock)) {
        /* Ok. */
        pBlock->Texture = pTexture->Name;
      }
    }    
  }
  
  /*============================================================================
  Sub-app: Texture selection tool
  ============================================================================*/
  void TextureSelectionTool::update(void) {
    getParent()->drawText( m_Pos,"Please select a texture from the list, or press ESC to cancel" );
    char cBuf[256];
    sprintf(cBuf,"(%d available textures)",m_List.size());
    getParent()->drawText( m_Pos+Vector2f(0,12),cBuf );

    /* Find out where the mouse is */
    int nX,nY;
    SDL_GetMouseState(&nX,&nY);
    
    /* Now draw all the famous textures... yir */
    Vector2f CurPos = m_Pos + Vector2f(5,29);
    Vector2f TexSize(50,50);
    
    for(int i=0;i<m_List.size();i++) {
      /* Next row? */
      if(CurPos.x + TexSize.x + 5 > m_Pos.x+m_Size.x) {
        CurPos.x = m_Pos.x + 5;
        CurPos.y += TexSize.y + 5;
      }  
      
      /* Draw it */
      getParent()->drawImage( CurPos,CurPos+TexSize,m_List[i] );
      
      if(nX >= CurPos.x && nY >= CurPos.y &&
         nX < CurPos.x+TexSize.x && nY < CurPos.y+TexSize.y) {
        getParent()->drawBox( CurPos-Vector2f(1,1),CurPos+TexSize+Vector2f(1,1),1,0,MAKE_COLOR(0,255,0,255) );        
        m_nHoverIdx = i;
      }
      
      /* Next column */
      CurPos.x += TexSize.x + 5;
    }
  }
  
  int TextureSelectionTool::getIdxByName(std::string Name) {
    for(int i=0;i<m_List.size();i++) 
      if(m_List[i]->Name == Name) return i;      
    return -1; /* name not recognized */
  }
  
  void TextureSelectionTool::keyDown(int nKey,int nChar) {
  }
  
  void TextureSelectionTool::keyUp(int nKey) {
  }
  
  void TextureSelectionTool::mouseDown(int nButton) {
    if(nButton == SDL_BUTTON_LEFT) {
      /* Finito */
      m_nSelectedIdx = m_nHoverIdx;
      subClose(0);
    }
  }
  
  void TextureSelectionTool::mouseUp(int nButton) {
  }

  /*============================================================================
  Sub-app: Parameter editor
  ============================================================================*/
  void ParamEditTool::update(void) {
		if(!m_bInit) {
			m_Menu.setName("");
			m_Menu.addItem("OK");
			m_Menu.addItem("RESET");
			m_Menu.addItem("CANCEL");
			m_bInit = true; /* don't do this again */
		}
		  
    int nX,nY;
    SDL_GetMouseState(&nX,&nY);

		getParent()->drawText(m_Pos,"Click on the parameter to change it:");
		m_nHoverIdx = -1;
		int k=0;
		for(int i=0;i<m_nNumEntries;i++) {
			if(nY >= m_Pos.y+14*i+10+20 && nY < m_Pos.y+14*i+10+20+12)
				m_nHoverIdx = i;
		
			getParent()->drawText(Vector2f(m_Pos.x+10,m_Pos.y+14*i+10+20),
			                      m_pTable[i].Name);
			                      
			if(m_nKeybFocus == i) {
				getParent()->drawText(Vector2f(m_Pos.x+140,m_Pos.y+14*i+10+20),
				                      std::string("[") + m_pTable[i].Value + std::string("]"),MAKE_COLOR(255,0,0,255),
				                      MAKE_COLOR(255,255,255,255));
			}
			else
				getParent()->drawText(Vector2f(m_Pos.x+140,m_Pos.y+14*i+10+20),
				                      std::string("[") + m_pTable[i].Value + std::string("]"),0,MAKE_COLOR(255,255,0,255));
  		k++;
		}				
		
		m_Menu.drawSimpleSelector((EditorApp *)getParent(),Vector2f(m_Pos.x+10,m_Pos.y+14*k+10+50));
  }
    
  void ParamEditTool::keyDown(int nKey,int nChar) {
		switch(nKey) {
			case SDLK_BACKSPACE:
				if(m_nKeybFocus>=0) {
					if(m_pTable[m_nKeybFocus].Value.length()>0) {
						m_pTable[m_nKeybFocus].Value.erase(m_pTable[m_nKeybFocus].Value.end()-1);
					}
				}
				break;
			case SDLK_RETURN:
				m_nKeybFocus = -1;
				break;
			case SDLK_UP:
				m_nKeybFocus--;
				if(m_nKeybFocus < 0) m_nKeybFocus = m_nNumEntries-1;
				break;
			case SDLK_DOWN:
				m_nKeybFocus++;
				if(m_nKeybFocus >= m_nNumEntries) m_nKeybFocus=0;
				break;
			case SDLK_ESCAPE:
				subClose(0); /* abort, cancel */
				break;
			default:
				/* Okay, focus? */
				if(m_nKeybFocus>=0) {				
					if(nChar) {
						char c[2];
						c[0]=nChar; c[1]='\0';
						m_pTable[m_nKeybFocus].Value.append(c);				
					}
				}
				break;
		}
  }
  
  void ParamEditTool::keyUp(int nKey) {
  }
  
  void ParamEditTool::mouseDown(int nButton) {
		if(nButton == SDL_BUTTON_LEFT) {
		  if(m_nHoverIdx<0) {
        int nX,nY;
        SDL_GetMouseState(&nX,&nY);

        if(nY >= m_Pos.y+14*m_nNumEntries+10+50 && nY <m_Pos.y+14*m_nNumEntries+10+62) {
		      if(m_Menu.getSelIdx() == 0) {
		        subClose(1);
		      }
		      else if(m_Menu.getSelIdx() == 1) {
		        /* Reset to original values */
		        for(int i=0;i<m_nNumEntries;i++)
		          m_pTable[i].Value = m_pTable[i].OrigValue;
		        m_nKeybFocus = -1;
		      }
		      else if(m_Menu.getSelIdx() == 2) {
		        subClose(0);
		      }
        }
      }
      else {
		    /* Set focus */
		    m_nKeybFocus = m_nHoverIdx;
      }
		}
  }
  
  void ParamEditTool::mouseUp(int nButton) {
  }
    
  /*============================================================================
  Sub-app: Input box
  ============================================================================*/
  void InputBox::update(void) {	  
		getParent()->drawText(m_Pos,m_Text);
		getParent()->drawText(m_Pos+Vector2f(0,12),"Press RETURN to accept and ESCAPE to cancel...");		
		getParent()->drawText(Vector2f(m_Pos.x+20,m_Pos.y+30),
				                      std::string("[") + m_Input + std::string("]"),MAKE_COLOR(255,0,0,255),
				                      MAKE_COLOR(255,255,255,255));      
  }
    
  void InputBox::keyDown(int nKey,int nChar) {
		switch(nKey) {
			case SDLK_BACKSPACE:
				if(m_Input.length()>0) 
					m_Input.erase(m_Input.end()-1);					
				break;
			case SDLK_RETURN:
			  subClose(1);
				break;
			case SDLK_ESCAPE:
				subClose(0); /* abort, cancel */
				break;
			default:
				if(nChar) {
					char c[2];
					c[0]=nChar; c[1]='\0';
					m_Input.append(c);				
				}
				break;
		}
  }
  
  void InputBox::keyUp(int nKey) {
  }
  
  void InputBox::mouseDown(int nButton) {
  }
  
  void InputBox::mouseUp(int nButton) {
  }
    
  /*============================================================================
  Edge smoothing
  ============================================================================*/
  void EditorApp::_SmoothSelectedEdges(void) {  
    while(1) {
      bool bS = false;
          
      for(int i=0;i<m_pLevelSrc->Blocks().size();i++) {
        for(int j=0;j<m_pLevelSrc->Blocks()[i]->Vertices.size();j++) {
          LevelBlockVertex *pVertex = m_pLevelSrc->Blocks()[i]->Vertices[j];
          if(pVertex->bSelected) {
            _SmoothEdge(m_pLevelSrc->Blocks()[i],pVertex,j);       
            bS = true;
          }
          
          if(bS) break;
        }
        if(bS) break;
      }    
      
      if(!bS) break;
    }
  }

  LevelBlockVertex *EditorApp::_NextVertex(LevelBlock *pBlock,int j,int *pn) {
    if(j + 1 < pBlock->Vertices.size()) {*pn = j+1; return pBlock->Vertices[j+1];}
    *pn = 0;
    return pBlock->Vertices[0];
  }
  
  LevelBlockVertex *EditorApp::_PrevVertex(LevelBlock *pBlock,int j,int *pn) {
    if(j - 1 < 0) {*pn = pBlock->Vertices.size()-1; return pBlock->Vertices[pBlock->Vertices.size()-1];}
    *pn = j-1;
    return pBlock->Vertices[j - 1];
  }  
  
  void EditorApp::_SmoothEdge(LevelBlock *pBlock,LevelBlockVertex *pEdge,unsigned int j) {
    /* Please don't read this (and the related ones) function! :D   I just
       caught my self thinking how this can be done in the easiest way possible 
       *Sigh* */

    int k,kk,z;  
    LevelBlockVertex *pPrevVertex = _PrevVertex(pBlock,j,&z);
    LevelBlockVertex *pVertex = pEdge;
    LevelBlockVertex *pNextVertex = _NextVertex(pBlock,j,&k);    
    LevelBlockVertex *pNextNextVertex = _NextVertex(pBlock,k,&kk);    
    
    Vector3f NewVertex;
    
    float fEdgeLen = Vector3f(pVertex->fX - pNextVertex->fX,pVertex->fY - pNextVertex->fY,0).length();
    
    Vector3f N1;
    N1 = Vector3f(pVertex->fX,pVertex->fY,0) - Vector3f(pPrevVertex->fX,pPrevVertex->fY,0);
    N1.normalize();        
    N1 = N1 * fEdgeLen * 0.25;    

    Vector3f N2;
    N2 = Vector3f(pNextVertex->fX,pNextVertex->fY,0) - Vector3f(pNextNextVertex->fX,pNextNextVertex->fY,0);
    N2.normalize();        
    N2 = N2 * fEdgeLen * 0.25;    
    
    /* This was indeed not the original intend of the bezier class, but it works
       here too. */
    BezierCurve Curve;
    
    Curve.setP(0,Vector3f(pVertex->fX,pVertex->fY,0));
    Curve.setP(1,Vector3f(pVertex->fX,pVertex->fY,0) + N1);
    Curve.setP(2,Vector3f(pNextVertex->fX,pNextVertex->fY,0) + N2);
    Curve.setP(3,Vector3f(pNextVertex->fX,pNextVertex->fY,0));
    
    /*NewVertex.x = (pVertex->fX + pNextVertex->fX)/2.0f;
    NewVertex.y = (pVertex->fY + pNextVertex->fY)/2.0f;*/

    NewVertex = Curve.step(0.5);    
    
    LevelBlockVertex *pNewVertex = new LevelBlockVertex;
    pNewVertex->EdgeEffect = pEdge->EdgeEffect;
    pNewVertex->bSelected = false;
    pNewVertex->fX = NewVertex.x;
    pNewVertex->fY = NewVertex.y;
    
    if(j+1 < pBlock->Vertices.size()) {
      pBlock->Vertices.insert(pBlock->Vertices.begin()+j+1,pNewVertex);
    }
    else
      pBlock->Vertices.push_back( pNewVertex );
      
    pEdge->bSelected = false;
  }

  /*============================================================================
  Entity snapping
  ============================================================================*/
  void EditorApp::_EntitySnapCursor(void) {
    Vector2f NearP;
    float fDist = 0;
    bool bGotPoint = false;
  
    for(int i=0;i<m_pLevelSrc->Entities().size();i++) {
      Vector2f P = Vector2f(m_pLevelSrc->Entities()[i]->fPosX,m_pLevelSrc->Entities()[i]->fPosY);
      float d = (P - m_Cursor).length();
      if(!bGotPoint || d<fDist) {
        NearP = P;
        fDist = d;
        bGotPoint = true;
      }                
    }

    if(bGotPoint)
      m_Cursor = NearP;
  }
   
  /*============================================================================
  Edge snapping
  ============================================================================*/
  void EditorApp::_EdgeSnapCursor(void) {
    Vector2f NearP;
    float fDist;
    bool bGotPoint = false;
  
    /* Find nearest edge, snap if it's close */
    for(unsigned int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      for(unsigned int j=0;j<m_pLevelSrc->Blocks()[i]->Vertices.size();j++) {
        int k;
        LevelBlockVertex *pVertex = m_pLevelSrc->Blocks()[i]->Vertices[j];
        LevelBlockVertex *pNextVertex = _NextVertex(m_pLevelSrc->Blocks()[i],j,&k);
        
        Vector2f u(pNextVertex->fX - pVertex->fX,pNextVertex->fY - pVertex->fY);
        Vector2f a(m_Cursor.x - (pVertex->fX + m_pLevelSrc->Blocks()[i]->fPosX),
                   m_Cursor.y - (pVertex->fY + m_pLevelSrc->Blocks()[i]->fPosY));
                                
        float zz = (a.dot(u) / (u.length() * u.length()));
        //printf("%f ",zz);
                
        if(zz >= 0.0f && zz < 1.0f) {
          Vector2f P = Vector2f(pVertex->fX+m_pLevelSrc->Blocks()[i]->fPosX,
                                pVertex->fY+m_pLevelSrc->Blocks()[i]->fPosY) + u*zz;
          //printf(" {%f} ",(P-m_Cursor).length());
          float d = (P-m_Cursor).length();
          if( d < 5 ) {
            if(!bGotPoint || d < fDist) {
              NearP = P;
              fDist = d;
              bGotPoint = true;
            }
          }
        }
      }
    }    
    
    if(bGotPoint)
      m_Cursor = NearP;
  }
  
  /*============================================================================
  Keyboard helpers
  ============================================================================*/
  bool EditorApp::_IsCtrlDown(void) {
    if(SDL_GetModState() & KMOD_CTRL) return true;
    return false;
  }
  
  bool EditorApp::_IsAltDown(void) {
    if(SDL_GetModState() & KMOD_ALT) return true;
    return false;
  }

  /*============================================================================
  Deleters
  ============================================================================*/
  void EditorApp::_DeleteSelectedBlocks(void) {    
    unsigned int i=0;
    unsigned int nDeleted = 0;
    while(1) {
      if(i >= m_pLevelSrc->Blocks().size()) break;

      if(_IsBlockSelected(m_pLevelSrc->Blocks()[i])) {
        for(unsigned int j=0;j<m_pLevelSrc->Blocks()[i]->Vertices.size();j++) {
          delete m_pLevelSrc->Blocks()[i]->Vertices[j];
        }
        delete m_pLevelSrc->Blocks()[i];
        m_pLevelSrc->Blocks().erase(m_pLevelSrc->Blocks().begin() + i);
        nDeleted++;      
      }
      else i++;
    }
    
    m_Log.msg("%d blocks deleted",nDeleted);
  }
  
  void EditorApp::_DeleteSelectedVertices(void) {
    unsigned int i=0;
    unsigned int nDeleted = 0,nDeletedObjects = 0;
    while(1) {
      if(i >= m_pLevelSrc->Blocks().size()) break;

      unsigned int j=0;
      while(1) {
        if(j >= m_pLevelSrc->Blocks()[i]->Vertices.size()) break;
        
        if(m_pLevelSrc->Blocks()[i]->Vertices[j]->bSelected) {
          delete m_pLevelSrc->Blocks()[i]->Vertices[j];
          m_pLevelSrc->Blocks()[i]->Vertices.erase(m_pLevelSrc->Blocks()[i]->Vertices.begin() + j);
          nDeleted++;
        }
        else j++;
      }
      
      if(m_pLevelSrc->Blocks()[i]->Vertices.size() < 3) {
        for(unsigned int j=0;j<m_pLevelSrc->Blocks()[i]->Vertices.size();j++) {
          delete m_pLevelSrc->Blocks()[i]->Vertices[j];
        }
        delete m_pLevelSrc->Blocks()[i];
        m_pLevelSrc->Blocks().erase(m_pLevelSrc->Blocks().begin() + i);
        nDeletedObjects++;
      }
      else i++;
    }    
    m_Log.msg("%d vertices deleted",nDeleted);
    if(nDeletedObjects>0) m_Log.msg("%d blocks deleted",nDeletedObjects);
  }
  
  void EditorApp::_DeleteSelectedEdges(void) {
    unsigned int i=0;
    unsigned int nDeleted = 0,nDeletedObjects = 0;
    bool bDeleteNext = false;
    while(1) {
      if(i >= m_pLevelSrc->Blocks().size()) break;

      unsigned int j=0;
      while(1) {
        if(j >= m_pLevelSrc->Blocks()[i]->Vertices.size()) break;
        
        if(m_pLevelSrc->Blocks()[i]->Vertices[j]->bSelected || bDeleteNext) {
          bool bS = m_pLevelSrc->Blocks()[i]->Vertices[j]->bSelected; 
          delete m_pLevelSrc->Blocks()[i]->Vertices[j];
          m_pLevelSrc->Blocks()[i]->Vertices.erase(m_pLevelSrc->Blocks()[i]->Vertices.begin() + j);
          
          if(bS) {
            nDeleted++;
            bDeleteNext = true;
            
            if(j == m_pLevelSrc->Blocks()[i]->Vertices.size() - 1) {
              delete m_pLevelSrc->Blocks()[i]->Vertices[0];
              m_pLevelSrc->Blocks()[i]->Vertices.erase(m_pLevelSrc->Blocks()[i]->Vertices.begin());              
              break;
            }
          }
          else
            bDeleteNext = false;
          
        }
        else j++;
      }
      
      if(m_pLevelSrc->Blocks()[i]->Vertices.size() < 3) {
        for(unsigned int j=0;j<m_pLevelSrc->Blocks()[j]->Vertices.size();j++) {
          delete m_pLevelSrc->Blocks()[i]->Vertices[j];
        }
        delete m_pLevelSrc->Blocks()[i];
        m_pLevelSrc->Blocks().erase(m_pLevelSrc->Blocks().begin() + i);
        nDeletedObjects++;
      }
      else i++;
    }    

    m_Log.msg("%d edges deleted",nDeleted);
    if(nDeletedObjects>0) m_Log.msg("%d blocks deleted",nDeletedObjects);
  }
  
  void EditorApp::_DeleteSelectedEntities(void) {
    unsigned int i=0;
    unsigned int nDeleted = 0;
    while(1) {
      if(i >= m_pLevelSrc->Entities().size()) break;
      
      if(m_pLevelSrc->Entities()[i]->bSelected) {
        for(unsigned int j=0;j<m_pLevelSrc->Entities()[i]->Params.size();j++)
          delete m_pLevelSrc->Entities()[i]->Params[j];
        
        delete m_pLevelSrc->Entities()[i];
        m_pLevelSrc->Entities().erase(m_pLevelSrc->Entities().begin() + i);
        nDeleted++;
      }
      else i++;
    }
    m_Log.msg("%d entities deleted",nDeleted);
  }

  /*============================================================================
  Toggle selected blocks background on/off
  ============================================================================*/
  void EditorApp::_ToggleSelectedBlockBackground(void) {
    for(unsigned int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
      if(_IsBlockSelected(pBlock)) {
        pBlock->bBackground = !pBlock->bBackground;
      }
    }        
  }

  /*============================================================================
  Toggle selected blocks water on/off
  ============================================================================*/
  void EditorApp::_ToggleSelectedBlockWater(void) {
    for(unsigned int i=0;i<m_pLevelSrc->Blocks().size();i++) {
      LevelBlock *pBlock = m_pLevelSrc->Blocks()[i];
      if(_IsBlockSelected(pBlock)) {
        pBlock->bWater = !pBlock->bWater;
      }
    }        
  }
      
}
