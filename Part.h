#ifndef PART_H
#define PART_H

#include "Shape.h"
#include "UndoList.h"

// forward declarations
class Net;
class CPartList;

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

#if 0
// struct to hold data to undo an operation on a part
//
class PartUndo : public UndoItem {
public:

private:
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
#endif

class Part
{
public:
	Part();
	~Part();

	int SetPartData( CShape * shape, QString ref_des, QString package,
					int x, int y, int side, int angle, int visible, int glued );
	int Highlight( );
	int MoveRefText(int x, int y, int angle, int size, int w );
	int MoveValueText(  int x, int y, int angle, int size, int w );
	void ResizeRefText(  int size, int width, bool vis=true );
	void ResizeValueText(  int size, int width, bool vis=true );
	void SetValue( QString value, int x, int y, int angle, int size, int w, bool vis=true );
	int Draw(  );
	void setVisible(bool bVisible );
	int SelectPart( );
	int SelectRefText( );
	int SelectValueText( );
	int SelectPad( int i );
	bool TestHitOnPad( QString pin_name, int x, int y, int layer );
	int Move(  int x, int y, int angle, int side );
	void PartFootprintChanged( CShape * shape );
	int GetPartBoundingRect(QRect & part_r );
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
	int GetPinLayer( QString pin_name );
	int GetPinLayer( int pin_index );
	Net * GetPinNet( QString pin_name );
	Net * GetPinNet( int pin_index );
	int GetPinWidth( QString pin_name );
	int StartDraggingPart( CDC * pDC, bool bRatlines,
								 bool bBelowPinCount, int pin_count );
	int StartDraggingRefText( CDC * pDC );
	int StartDraggingValue( CDC * pDC );
	int StopDragging();
	int CancelDraggingPart();
	int CancelDraggingRefText();
	int CancelDraggingValue();
	int SetPartString(QString str );
	int GetPinConnectionStatus( QString pin_name, int layer );

//	undo_part * CreatePartUndoRecord(CString * new_ref_des );
//	static void PartUndoCallback( int type, void * ptr, bool undo );




private:
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

#endif // PART_H
