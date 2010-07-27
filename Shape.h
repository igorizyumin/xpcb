// Shape.h : interface for the CShape class
//
#pragma once

#include "PolyLine.h"
#include "smfontutil.h"

class CTextList;

#define CENTROID_WIDTH 40*NM_PER_MIL	// width of centroid symbol
#define DEFAULT_GLUE_WIDTH 15*NM_PER_MIL	// width of default glue spot

// pad shapes
enum {
	PAD_NONE = 0,
	PAD_ROUND,
	PAD_SQUARE,
	PAD_RECT,
	PAD_RRECT,
	PAD_OVAL,
	PAD_OCTAGON,
	PAD_DEFAULT = 99
};

// pad area connect flags
enum {
	PAD_CONNECT_DEFAULT = 0,
	PAD_CONNECT_NEVER,
	PAD_CONNECT_THERMAL,
	PAD_CONNECT_NOTHERMAL
};

// error returns
enum
{
	PART_NOERR = 0,
	PART_ERR_TOO_MANY_PINS
};

// centroid types
enum CENTROID_TYPE
{
	CENTROID_DEFAULT = 0,	// center of pads
	CENTROID_DEFINED		// defined by user
};

// glue spot position types 
enum GLUE_POS_TYPE
{
	GLUE_POS_CENTROID,	// at centroid
	GLUE_POS_DEFINED	// defined by user
};

// structure describing pad flags
struct Flag
{
	unsigned int mask : 1;
	unsigned int area : 2;
};

// structure describing adhesive spot
struct Glue
{
	GLUE_POS_TYPE type;
	int w, x_rel, y_rel;
};

// structure describing stroke (ie. line segment)
struct Stroke
{
	int w, xi, yi, xf, yf;	// thickness + endpoints
	int type;				// CDisplayList g_type
};

// structure describing mounting hole
// only used during conversion of Ivex files
struct Mtg_Hole
{
	int pad_shape;		// used for pad on top and bottom
	int x, y, diam, pad_diam;
};


// structure describing pad
class Pad
{
public:
	Pad();
	bool operator==(Pad p);
	int shape;	// see enum above
	int size_l, size_r, size_h, radius;
	int connect_flag;	// only for copper pads
};

// padstack is pads and hole associated with a pin
class Padstack
{
public:
	Padstack();
	bool operator==(Padstack p);
	bool exists;		// only used when converting Ivex footprints or editing
	QString name;		// identifier such as "1" or "B24"
	int hole_size;		// 0 = no hole (i.e SMT)
	int x_rel, y_rel;	// position relative to part origin
	int angle;			// orientation: 0=left, 90=top, 180=right, 270=bottom
	Pad top, top_mask, top_paste;
	Pad bottom, bottom_mask, bottom_paste;
	Pad inner;
};

// CShape class represents a footprint
//
class Footprint
{
	// if variables are added, remember to modify Copy!
private:
	enum { MAX_NAME_SIZE = 59 };	// max. characters
	enum { MAX_PIN_NAME_SIZE = 39 };
	enum { MAX_VALUE_SIZE = 39 };
	QString m_name;		// name of shape (e.g. "DIP20")
	QString m_author;
	QString m_source;
	QString m_desc;
	int m_units;		// units used for original definition (MM, NM or MIL)
	int m_sel_xi, m_sel_yi, m_sel_xf, m_sel_yf;			// selection rectangle
	int m_ref_size, m_ref_xi, m_ref_yi, m_ref_angle;	// ref text
	int m_ref_w;						// thickness of stroke for ref text
	int m_value_size, m_value_xi, m_value_yi, m_value_angle;	// value text
	int m_value_w;						// thickness of stroke for value text
	CENTROID_TYPE m_centroid_type;		// type of centroid
	int m_centroid_x, m_centroid_y;		// position of centroid
	int m_centroid_angle;				// angle of centroid (CCW)
	QVector<Padstack> m_padstack;		// array of padstacks for shape
	QVector<CPolyLine> m_outline_poly;	// array of polylines for part outline
	CTextList * m_tl;					// list of text strings
	QVector<Glue> m_glue;		// array of adhesive dots

public:
	Footprint();
	~Footprint();
	void Clear();
	int MakeFromString( QString name, QString str );
	int MakeFromFile( QFile & in_file, QString name, QString file_path, int pos );
	int WriteFootprint( QFile & file );
	int GetNumPins();
	int GetPinIndex( QString name );
	QString GetPinName( int index );
	QRect GetBounds( bool bIncludeLineWidths=true );
	QRect GetCornerBounds();
	QRect GetPadBounds( int i );
	QRect GetPadRowBounds( int i, int num );
	QPoint GetDefaultCentroid();
	QRect GetAllPadBounds();
	int Copy( Footprint * shape );	// copy all data from shape
	bool Compare( Footprint * shape );	// compare shapes, return true if same

	Padstack getPadstack(int i) { return m_padstack[i]; }
	Padstack getPadstack(QString name) { return getPadstack(GetPinIndex(name)); }
};

#if 0
// CEditShape class represents a footprint whose elements can be edited
//
class CEditShape : public CShape
{
public:
	CEditShape();
	~CEditShape();
	void Clear();
	void Draw( CDisplayList * dlist, SMFontUtil * fontutil );
	void Undraw();
	void Copy( CShape * shape );
	void SelectPad( int i );
	void StartDraggingPad( CDC * pDC, int i );
	void CancelDraggingPad( int i );
	void StartDraggingPadRow( CDC * pDC, int i, int num );
	void CancelDraggingPadRow( int i, int num );
	void SelectRef();
	void StartDraggingRef( CDC * pDC );
	void CancelDraggingRef();
	void SelectValue();
	void StartDraggingValue( CDC * pDC );
	void CancelDraggingValue();
	void SelectAdhesive( int idot );
	void StartDraggingAdhesive( CDC * pDC, int idot );
	void CancelDraggingAdhesive( int idot );
	void SelectCentroid();
	void StartDraggingCentroid( CDC * pDC );
	void CancelDraggingCentroid();
	void ShiftToInsertPadName( CString * astr, int n );
	bool GenerateSelectionRectangle( CRect * r );

public:
	CDisplayList * m_dlist;
	CArray<dl_element*> m_hole_el;		// hole display element 
	CArray<dl_element*> m_pad_top_el;		// top pad display element 
	CArray<dl_element*> m_pad_inner_el;		// inner pad display element 
	CArray<dl_element*> m_pad_bottom_el;	// bottom pad display element 
	CArray<dl_element*> m_pad_top_mask_el;
	CArray<dl_element*> m_pad_top_paste_el;
	CArray<dl_element*> m_pad_bottom_mask_el;
	CArray<dl_element*> m_pad_bottom_paste_el;
	CArray<dl_element*> m_pad_sel;		// pad selector
	CArray<dl_element*> m_ref_el;		// strokes for "REF"
	dl_element * m_ref_sel;				// ref selector
	CArray<dl_element*> m_value_el;		// strokes for "VALUE"
	dl_element * m_value_sel;			// value selector
	dl_element * m_centroid_el;			// centroid
	dl_element * m_centroid_sel;		// centroid selector
	CArray<dl_element*> m_dot_el;		// adhesive dots
	CArray<dl_element*> m_dot_sel;		// adhesive dot selectors
};
#endif
