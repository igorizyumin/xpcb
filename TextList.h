// textlist.cpp ... definition of class CTextList
//
#pragma once

#include "smfontutil.h"
#include "UndoList.h"

class CTextList;
struct Stroke;

struct undo_text {
	GUID m_guid;
	int m_x, m_y;
	int m_Stroke
	int m_angle;
	BOOL m_mirror;
	BOOL m_bNegative;
	int m_font_size;
	int m_stroke_width;
	CString m_str;
	int m_nstrokes;
	CTextList * m_tlist;
};



class CStrokest
{
public:
	enum {
		UNDO_TEXT_ADD = 1,
		UNDO_TEXT_MODIFY,
		UNDO_TEXT_DELETE
	};
	// member functions
	CTextList();
	CTextList( CDisplayList * dlist, SMFontUtil * smfontutil );
	~CTextList();
	Text * AddText( int x, int y, int angle, int mirror,
					BOOL bNegative,	int layer, 
					int font_size, int stroke_width, 
					CString * str_ptr, BOOL draw_flag=TRUE );
	int RemoveText( Text * text );
	void  RemoveAllTexts();
	void HighlightText( Text * text );
	void StartDraggingText( CDC * pDC, Text * text );
	void CancelDraggingText( Text * text );
	void MoveText( Text * text, int x, int y, int angle,
		BOOL mirror, BOOL negative, int layer );
	void ReadTexts( CStdioFile * file );
	int WriteTexts( CStdioFile * file );
	void MoveOrigin( int x_off, int y_off );
	Text * GetText( GUID * guid );
	Text * GetFirstText();
	Text * GetNextText();
	int GetNumTexts(){ return text_ptr.GetSize();};
	BOOL GetTextBoundaries( CRect * r );
	BOOL GetTextRectOnPCB( Text * t, CRect * r );
	void ReassignCopperLayers( int n_new_layers, int * layer );
	undo_text * CreateUndoRecord( Text * text );
	static void TextUndoCallback( int type, void * ptr, BOOL undo );

	// member variables
	SMFontUtil * m_smfontutil;
	CDisplayList * m_dlist;
	CArray<Text*> text_ptr;
};

