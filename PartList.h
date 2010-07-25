
#pragma once

#include "Shape.h"
#include "smfontutil.h"
#include "UndoList.h"

#define MAX_REF_DES_SIZE 39

class Part;
class CPartList;
class CNetList;
class Net;

#include "DesignRules.h"

// clearance types for GetPadDrawInfo()
enum {
	CLEAR_NORMAL = 0,
	CLEAR_THERMAL,
	CLEAR_NONE
};

// struct to hold data to undo an operation on a part
//
struct undo_part {
	int size;				// size of this instance of the struct
	id m_id;				// instance id for this part
	bool visible;			// false=hide part
	int x,y;				// position of part origin on board 
	int side;				// 0=top, 1=bottom
	int angle;				// orientation (degrees)
	bool glued;				// true=glued in place
	bool m_ref_vis;			// true = ref shown
	int m_ref_xi, m_ref_yi, m_ref_angle, m_ref_size, m_ref_w;	// ref text
	bool m_value_vis;		// true = value shown
	int m_value_xi, m_value_yi, m_value_angle, m_value_size, m_value_w;	// value text
	char ref_des[MAX_REF_DES_SIZE+1];	// ref designator such as "U3"
	char new_ref_des[MAX_REF_DES_SIZE+1];	// if ref designator will be changed
	char package[CShape::MAX_NAME_SIZE+1];		// package
	char value[CShape::MAX_VALUE_SIZE+1];		// package
	char shape_name[CShape::MAX_NAME_SIZE+1];	// name of shape
	CShape * shape;			// pointer to the footprint of the part, may be NULL
	CPartList * m_plist;	// parent cpartlist	
	// here goes array of char[npins][40] for attached net names
};

// partlist_info is used to hold digest of CPartList 
// for editing in dialogs, or importing from netlist file
// notes:
//	package may be "" if no package assigned
//	shape may be NULL if no footprint assigned
//	may have package but no footprint, but not the reverse
typedef struct {
	Part * part;		// pointer to original part, or NULL if new part added
	CString ref_des;	// ref designator string
	int ref_size;		// size of ref text characters
	int ref_width;		// stroke width of ref text characters
	CString package;	// package (from original imported netlist, don't edit)
	CString value;		// value (from original imported netlist, don't edit)
	bool value_vis;		// visibility of value
	CShape * shape;		// pointer to shape (may be edited)
	bool deleted;		// flag to indicate that part was deleted
	bool bShapeChanged;	// flag to indicate that the shape has changed
	bool bOffBoard;		// flag to indicate that position has not been set
	int x, y;			// position (may be edited)
	int angle, side;	// angle and side (may be edited)
} part_info;

typedef CArray<part_info> partlist_info;

// error codes
enum
{
	PL_NOERR = 0,
	PL_NO_DLIST,
	PL_NO_FOOTPRINT,
	PL_ERR
};

// struct used for DRC to store pin info
struct drc_pin {
	int hole_size;	// hole diameter or 0
	int min_x;		// bounding rect of padstack
	int max_x;
	int min_y;
	int max_y;
	int max_r;		// max. radius of padstack
	int layers;		// bit mask of layers with pads
};

// class part_pin represents a pin on a part
// note that pin numbers start at 1,
// so index to pin array is (pin_num-1)
class PartPin
{
public:
	int x, y;				// position on PCB
	Net * net;				// pointer to net, or NULL if not assigned
	drc_pin drc;			// drc info
};

class Part
{
public:
	Part();
	~Part();

	int SetPartData( CShape * shape, CString * ref_des, CString * package,
					int x, int y, int side, int angle, int visible, int glued );
	int Highlight( );
	int MoveRefText(int x, int y, int angle, int size, int w );
	int MoveValueText(  int x, int y, int angle, int size, int w );
	void ResizeRefText(  int size, int width, bool vis=true );
	void ResizeValueText(  int size, int width, bool vis=true );
	void SetValue( CString * value, int x, int y, int angle, int size, int w, bool vis=true );
	int Draw(  );
	void setVisible(bool bVisible );
	int SelectPart( );
	int SelectRefText( );
	int SelectValueText( );
	int SelectPad( int i );
	bool TestHitOnPad( CString * pin_name, int x, int y, int layer );
	int Move(  int x, int y, int angle, int side );
	void PartFootprintChanged( CShape * shape );
	int GetPartBoundingRect(CRect * part_r );
	int GetSide();
	int GetAngle();
	int GetRefAngle();
	int GetValueAngle();
	CPoint GetRefPoint();
	CPoint GetValuePoint();
	CRect GetValueRect();
	CPoint GetPinPoint( QString pin_name );
	CPoint GetPinPoint( int pin_index );
	CPoint GetCentroidPoint();
	CPoint GetGluePoint( int iglue );
	int GetPinLayer( CString * pin_name );
	int GetPinLayer( int pin_index );
	Net * GetPinNet( CString * pin_name );
	Net * GetPinNet( int pin_index );
	int GetPinWidth( CString * pin_name );
	int StartDraggingPart( CDC * pDC, bool bRatlines,
								 bool bBelowPinCount, int pin_count );
	int StartDraggingRefText( CDC * pDC );
	int StartDraggingValue( CDC * pDC );
	int StopDragging();
	int CancelDraggingPart();
	int CancelDraggingRefText();
	int CancelDraggingValue();
	int SetPartString(CString * str );
	int GetPinConnectionStatus( CString * pin_name, int layer );

	undo_part * CreatePartUndoRecord(CString * new_ref_des );
	static void PartUndoCallback( int type, void * ptr, bool undo );




private:
	int RotatePoint( CPoint * p, int angle, CPoint org );
	int RotateRect( CRect * r, int angle, CPoint p );

	id m_id;			// instance id for this part
	bool drawn;			// true if part has been drawn to display list
	bool visible;		// 0 to hide part
	int x,y;			// position of part origin on board
	int side;			// 0=top, 1=bottom
	int angle;			// orientation
	bool glued;			// 1=glued in place
	bool m_ref_vis;		// true = ref shown
	int m_ref_xi;		// reference text (relative to part)
	int m_ref_yi;	
	int m_ref_angle; 
	int m_ref_size;
	int m_ref_w;
	bool m_value_vis;	// true = value shown
	int m_value_xi;		// value text
	int m_value_yi; 
	int m_value_angle; 
	int m_value_size; 
	int m_value_w;		
	QString ref_des;			// ref designator such as "U3"
	QString value;				// "value" string
	QString package;			// package (from original imported netlist, may be "")
	CShape * shape;				// pointer to the footprint of the part, may be NULL
	QList<stroke> ref_text_stroke;		// strokes for ref. text
	QList<stroke> value_stroke;		// strokes for ref. text
	QList<stroke> m_outline_stroke;	// array of outline strokes
	QList<PartPin> pin;				// array of all pins in part
	int utility;		// used for various temporary purposes
	// drc info
	bool hole_flag;	// true if holes present
	int min_x;		// bounding rect of pads
	int max_x;
	int min_y;
	int max_y;
	int max_r;		// max. radius of pads
	int layers;		// bit mask for layers with pads
	// flag used for importing
	bool bPreserve;	// preserve connections to this part
};

class CPartList
{
public:
	enum {
		NOT_CONNECTED = 0,		// pin not attached to net
		ON_NET = 1,				// pin is attached to a net
		TRACE_CONNECT = 2,		// pin connects to trace on this layer
		AREA_CONNECT = 4		// pin connects to copper area on this layer
	};

	// TODO replace these with methods that return iterators for QList
	Part m_start, m_end;

public:
	enum { 
		UNDO_PART_DELETE=1, 
		UNDO_PART_MODIFY, 
		UNDO_PART_ADD };	// undo types

	CPartList( SMFontUtil * fontutil );
	~CPartList();


	// getter/setter functions
	int GetNumParts(){ return m_parts.size(); }
	void SetShapeCacheMap( QMap<QString, CShape*> * shape_cache_map ) { m_footprint_cache_map = shape_cache_map; }
	void SetNetlist( CNetList * nlist ){ m_nlist = nlist; }
	void SetNumCopperLayers( int nlayers ){ m_layers = nlayers;}
	void SetPinAnnularRing( int ring ){ m_annular_ring = ring; }
	int GetPartBoundaries( CRect * part_r );
	int GetNumFootprintInstances( CShape * shape );

	// container functions
	Part * Add();
	Part * Add( CShape * shape, CString * ref_des, CString * package,
					int x, int y, int side, int angle, int visible, int glued ); 
	Part * AddFromString( CString * str );
	int Remove( Part * element );
	void RemoveAllParts();
	Part * GetPart( QString ref_des );


	void MarkAllParts( int mark );
	void HighlightAllPadsOnNet( Net * net );
	int GetPadDrawInfo( Part * part, int ipin, int layer,
							  bool bUse_TH_thermals, bool bUse_SMT_thermals,
							  int mask_clearance, int paste_mask_shrink,
							  int * type=0, int * x=0, int * y=0, int * w=0, int * l=0, int * r=0, int * hole=0,
							  int * angle=0, Net ** net=0,
							  int * connection_status=0, int * pad_connect_flag=0,
							  int * clearance_type=0 );




	void MoveOrigin( int x_off, int y_off );


	void FootprintChanged( CShape * shape );
	void RefTextSizeChanged( CShape * shape );

	int WriteParts( CStdioFile * file );
	int ReadParts( CStdioFile * file );
	int ExportPartListInfo( partlist_info * pl, Part * part );
	void ImportPartListInfo( partlist_info * pl, int flags, CDlgLog * log=NULL );


	void PurgeFootprintCache();

	// check functions
	int CheckPartlist( CString * logstr );
	bool CheckForProblemFootprints();
	void DRC( CDlgLog * log, int copper_layers, 
		int units, bool check_unrouted,
		CArray<CPolyLine> * board_outline,
		DesignRules * dr, DRErrorList * DRElist );

private:
	QList<Part> m_parts;	// the actual list of parts
	int m_max_size;
	int m_layers;
	int m_annular_ring;
	CNetList * m_nlist;
	SMFontUtil * m_fontutil;	// class for Hershey font
	QMap<QString,CShape*> * m_footprint_cache_map;


};
 
