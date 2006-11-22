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

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "VApp.h"
#include "xmscene/Level.h"
#include "VTexture.h"

#include "EditorData.h"
#include "EditorLog.h"

#include "xmscene/Block.h"
#include "xmscene/Level.h"
#include "xmscene/Entity.h"

namespace vapp {
  
  class EditorApp;
  
	/*===========================================================================
	Click targets (stupid name, but it's late at night :P)
  ===========================================================================*/
  enum ClickTarget {
    CT_NOTHING,
    CT_SELECTION,
    CT_SINGLE_OBJECT
  };

	/*===========================================================================
	Selection modes
  ===========================================================================*/
  enum SelectionMode {
    SM_VERTICES,
    SM_EDGES,
    SM_BLOCKS,
    SM_ENTITIES
  };
  
	/*===========================================================================
	States
  ===========================================================================*/
  enum EditorState {
    ES_DEFAULT,
    ES_MOVING_LIMITS,
    ES_CREATING_NEW_BLOCK,
    ES_SAVING_LEVEL,
    ES_SELECTING,
    ES_MOVING_SELECTION,
    ES_CREATING_NEW_ENTITY,
    ES_EDITING_SELECTED_ENTITY,
    ES_EDITING_LEVEL_PROPS,
    ES_EDITING_EDGE_EFFECT,
    ES_CREATING_NEW_LEVEL,
    ES_LOADING_LEVEL,
    ES_SMOOTHING_EDGES,
    ES_PLAYING_LEVEL,
    ES_DELETING_SELECTION,
    ES_TOGGLE_BG_BLOCK,
    ES_COPYING_ENTITY,
    ES_TOGGLE_WATER_BLOCK
  };

	/*===========================================================================
	Simple selection menu
  ===========================================================================*/
  class EditorSimpleSelector {
    public:
      EditorSimpleSelector() {m_nSelIdx=0;}
    
      void drawSimpleSelector(EditorApp *pEditor,const Vector2f &Pos);
      float getWidth(void);
    
      /* Data interface */
      void setName(std::string s) {m_Name=s;}
      void addItem(std::string s) {m_Items.push_back(s);}
      int getSelIdx(void) {return m_nSelIdx;}
      void setSelIdx(int n) {m_nSelIdx = n;}
      
    private:          
      /* Data */
      std::string m_Name;                 /* Name of menu */
      std::vector<std::string> m_Items;   /* List of items */
      int m_nSelIdx;                      /* Index of selection */
  };

	/*===========================================================================
	Texture selection tool
  ===========================================================================*/    
  class TextureSelectionTool : public SubApp {
   public:
      TextureSelectionTool() {
        m_nHoverIdx=m_nSelectedIdx=-1;
      }
    
      /* Virtuals */
      virtual void update(void);
      virtual void keyDown(int nKey,int nChar);
      virtual void keyUp(int nKey);
      virtual void mouseDown(int nButton);
      virtual void mouseUp(int nButton);          
      
      /* Data interface */
      void setList(std::vector<Texture *> &List) {m_List = List;}      
      void setDims(Vector2f Pos,Vector2f Size) {m_Pos = Pos; m_Size = Size;}
      int getSelectedIdx(void) {return m_nSelectedIdx;}
      void setSelectedIdx(int nIdx) {m_nSelectedIdx=nIdx;}
      int getIdxByName(std::string Name);
    
    private:
      /* Data */
      std::vector<Texture *> m_List;
      Vector2f m_Pos,m_Size;
      int m_nHoverIdx,m_nSelectedIdx;
  };

	/*===========================================================================
	Input box
  ===========================================================================*/    
  class InputBox : public SubApp {
		public:
			InputBox() {m_Text = m_Input = "";}
		
			/* Virtuals */
      virtual void update(void);
      virtual void keyDown(int nKey,int nChar);
      virtual void keyUp(int nKey);
      virtual void mouseDown(int nButton);
      virtual void mouseUp(int nButton);          

			/* Data interface */
      void setDims(Vector2f Pos,Vector2f Size) {m_Pos = Pos; m_Size = Size;}
      void setInit(std::string Text,std::string Input) {m_Text=Text; m_Input=Input;}
      std::string getInput(void) {return m_Input;}
		
		private:
			/* Data */
			std::string m_Text;
			std::string m_Input;
      Vector2f m_Pos,m_Size;
  };
  
	/*===========================================================================
	Parameter editing tool -- entry
  ===========================================================================*/    
  struct PETEntry {
		std::string Name,Value,OrigValue;
  };
  
	/*===========================================================================
	Parameter editing tool
  ===========================================================================*/    
  class ParamEditTool : public SubApp {
		public:
			ParamEditTool() {m_nNumEntries=m_nHoverIdx=m_nKeybFocus=-1; m_bInit=false;}
		
			/* Virtuals */
      virtual void update(void);
      virtual void keyDown(int nKey,int nChar);
      virtual void keyUp(int nKey);
      virtual void mouseDown(int nButton);
      virtual void mouseUp(int nButton);          

			/* Data interface */
      void setDims(Vector2f Pos,Vector2f Size) {m_Pos = Pos; m_Size = Size;}
      void setTable(PETEntry *pTable,int nNumEntries) {m_pTable = pTable; m_nNumEntries=nNumEntries;}
		
		private:
			/* Data */
			PETEntry *m_pTable;
			int m_nNumEntries,m_nHoverIdx;
			int m_nKeybFocus;
      Vector2f m_Pos,m_Size;
      EditorSimpleSelector m_Menu;
      bool m_bInit;
  };

	/*===========================================================================
	Menu
  ===========================================================================*/
  class EditorMenu {
    public:
    EditorMenu() {}
    virtual ~EditorMenu() {}

      /* Virtual methods */
      virtual void itemSelected(int nItem) {}
    
      /* Draw */
      void drawMenu(EditorApp *pEditor,const Vector2f &Pos);

      /* Data interface */
      void addItem(std::string Item) {m_Items.push_back( Item );}
      
    private:
      /* Data */
      std::vector<std::string> m_Items;
  };

	/*===========================================================================
	Main menu
  ===========================================================================*/
  class EditorMainMenu : public EditorMenu {
    public:
      EditorMainMenu() {}
      EditorMainMenu(EditorApp *pEditor) {m_pEditor=pEditor;}
      virtual ~EditorMainMenu() {}    

      /* Virtual methods */
      virtual void itemSelected(int nItem);
                
    private:
      /* Data */
      EditorApp *m_pEditor;
  };

	/*===========================================================================
	Entity menu
  ===========================================================================*/
  class EditorEntityMenu : public EditorMenu {
    public:
      EditorEntityMenu() {}
      EditorEntityMenu(EditorApp *pEditor) {m_pEditor=pEditor;}
      virtual ~EditorEntityMenu() {}

      /* Virtual methods */
      virtual void itemSelected(int nItem);
                
    private:
      /* Data */
      EditorApp *m_pEditor;
  };

	/*===========================================================================
	Editor application
  ===========================================================================*/
  class EditorApp : public App {
    public:
      EditorApp();
    
      /* Virtual methods */
      virtual void drawFrame(void);
      virtual void userInit(void);
      virtual void userShutdown(void);
      virtual void keyDown(int nKey,int nChar);
      virtual void keyUp(int nKey);
      virtual void mouseDown(int nButton);
      virtual void mouseUp(int nButton);
      virtual void parseUserArgs(std::vector<std::string> &UserArgs);
      
      /* Design view drawing/stuffin' methods */
      void setViewRect(Vector2f Min,Vector2f Max);
      void setViewPoint(Vector2f Point);
      void moveView(Vector2f Dir);
      void zoomView(float fFactor);
      void resetView(void);
      
      void viewDrawLine(Vector2f P0,Vector2f P1,Color Col);
      void viewDrawCircle(Vector2f Cp,float Cr,float fBorder,Color BgColor,Color FgColor);
      void viewVertex(Vector2f V);
      void viewDrawText(Vector2f V,std::string Text,Color BgColor,Color FgColor);
      void viewDrawGrid(void);
      void viewMessage(std::string Text);
      
      /* State changing */
      void setState(EditorState s,int nOptParam=0);
      
      /* Misc */
      bool isLeftButtonClicked(void) {return m_bLeftButtonClick;}
    
    private: 
      /* Data */
      bool m_bJustPackage;
      
      Vector2f m_ViewMin,m_ViewMax;           /* Design viewport */
      EditorMainMenu *m_pMainMenu;            /* Main menu */
      EditorEntityMenu *m_pEntityMenu;				/* Entity menu */
      Level *m_pLevelSrc;                  /* Level */
      
      std::string m_LevelToInitLoad;          /* Load this level for editing */
      
      EditorSimpleSelector m_SnapSelect;
      EditorSimpleSelector m_SelStyleSelect;
      
      Vector2f m_Cursor;                      /* Cursor position in level coordinates */
      Vector2f m_PrevCursor;                  /* Previous cursor pos */
      bool m_bCursorInViewport;               /* Cursor is right now situated in the view */
      int m_nGridSnap;                        /* Grid snap */
      EditorState m_CurState;                 /* Current editor state */
      SelectionMode m_SelMode;                /* Current selection mode */
      
      bool m_bLeftButtonDown,m_bLeftButtonClick;
      bool m_bTextureBrowserMouseHover;
      
      Entity *m_pEntityToCopy;
           
      EditorData m_EntityTable;								/* Available entities */
      EditorLog m_Log;												/* Log object */
            
      /* GUI elements */
      Vector2f m_MainMenuAreaPos,m_MainMenuAreaSize;
      Vector2f m_ViewportPos,m_ViewportSize;      
      Vector2f m_TextureBrowserPos,m_TextureBrowserSize;
      int m_nLogLines;
      
      /* When ES_CREATING_NEW_BLOCK */      
      std::vector<float> m_NewBlockX,m_NewBlockY;       
      
      /* When ES_SELECTING */
      Vector2f m_SelPoint;
      
      /* When ES_MOVING_SELECTION */
      Vector2f m_MStart;
      
      /* When ES_CREATING_NEW_ENTITY */
      std::string m_NewEntityID;
      
      /* theme */
      ThemeChoicer *m_themeChoicer;

      /* Helper functions */
      void _NewLevel(void);
      float _GetAvg(std::vector<float> &x);
      std::string _CreateBlockName(void);
      std::string _CreateZoneName(void);
      void _SelectByBox(void);
      ClickTarget _AnythingAtPoint(Vector2f P,bool bSelectAtWill);
      void _ClearSelection(void);
      void _FinishMovement(Vector2f Dir);
      bool _IsBlockSelected(Block *pBlock);
      void _DrawTextureBrowser(void);
      Texture *_GetCommonTexture(std::vector<Block *> &Blocks);       
      void _AssignTextureToSelection(Texture *pTexture);    
      void _SelectBlock(Block *pBlock); 
      void _CreateEntityAtPos(std::string TypeID,Vector2f Pos);
      void _CopyEntityAtPos(Entity *pToCopy,Vector2f Pos);
      void _DrawEntitySymbol(Entity *pEntity);
      Entity *_GetSelectedEntity(void);
      std::string _GetCommonEdgeEffect(int *pnNumSel);
      void _ApplyEdgeEffect(std::string Effect);
      void _SmoothSelectedEdges(void);
      void _SmoothEdge(Block *pBlock,BlockVertex *pEdge,unsigned int j);
      BlockVertex *_NextVertex(Block *pBlock,int j,int *pn);
      BlockVertex *_PrevVertex(Block *pBlock,int j,int *pn);
      void _EdgeSnapCursor(void);
      void _EntitySnapCursor(void);
      void _DeleteSelectedBlocks(void);
      void _DeleteSelectedVertices(void);
      void _DeleteSelectedEdges(void);
      void _DeleteSelectedEntities(void);
      void _ToggleSelectedBlockBackground(void);
      void _ToggleSelectedBlockWater(void);
      
      /* Keyboard input */
      bool _IsCtrlDown(void);
      bool _IsAltDown(void);
  };

}

#endif

