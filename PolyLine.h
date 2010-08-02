// PolyLine.h ... definition of CPolyLine class
//
// A polyline contains one or more contours, where each contour
// is defined by a list of corners and side-styles
// There may be multiple contours in a polyline.
// The last contour may be open or closed, any others must be closed.
// All of the corners and side-styles are concatenated into 2 arrays,
// separated by setting the end_contour flag of the last corner of 
// each contour.
//
// When used for copper areas, the first contour is the outer edge 
// of the area, subsequent ones are "holes" in the copper.
//
// If a CDisplayList pointer is provided, the polyline can draw itself 

#pragma once

#include "global.h"
#include "PCBObject.h"
#include <QList>
#include <QPainter>


class gpc_polygon;
class gpc_op;


class CArc {
public: 
	enum{ MAX_STEP = 50*25400 };	// max step is 20 mils
	enum{ MIN_STEPS = 18 };		// min step is 5 degrees
	int style;
	int xi, yi, xf, yf;
	int n_steps;	// number of straight-line segments in gpc_poly 
	bool bFound;
};

class CPolyPt
{
public:
	CPolyPt( int qx=0, int qy=0, bool qf=false ):
			x(qx), y(qy), end_contour(qf), utility(0) {}
	int x;
	int y;
	bool end_contour;
	int utility;
};

class CPolyLine : public PCBObject
{
public:
	enum { STRAIGHT, ARC_CW, ARC_CCW };	// side styles
	enum { NO_HATCH, DIAGONAL_FULL, DIAGONAL_EDGE }; // hatch styles
	enum { DEF_SIZE = 50, DEF_ADD = 50 };	// number of array elements to add at a time

	// constructors/destructor
	CPolyLine();
	~CPolyLine();

	// functions for modifying polyline
	void Start( int layer, int w, int sel_box, int x, int y,
		int hatch);
	void AppendCorner( int x, int y, int style = STRAIGHT );
	void InsertCorner( int ic, int x, int y );
	void DeleteCorner( int ic, bool bDraw=true );
	void MoveCorner( int ic, int x, int y );
	void Close( int style = STRAIGHT, bool bDraw=true );
	void RemoveContour( int icont );

	// clipping/combination
	int TestSelfIntersection();
	int Clip(bool bRetainArcs=true );
	int TestIntersection( const CPolyLine &other );
	int Combine( CPolyLine *other );

	// drawing functions
	// XXX move this to editor delegate
	void HighlightSide( int is );
	void HighlightCorner( int ic );
	void StartDraggingToInsertCorner( QPainter *painter, int ic, int x, int y, int crosshair = 1 );
	void StartDraggingToMoveCorner( QPainter *painter, int ic, int x, int y, int crosshair = 1 );
	void CancelDraggingToInsertCorner( int ic );
	void CancelDraggingToMoveCorner( int ic );
	virtual void draw( QPainter *painter, PCBLAYER layer);

	void SetVisible( bool visible = true );
	void MoveOrigin( int x_off, int y_off );
	void SetSideVisible( int is, int visible );

	// misc. functions
	QRect GetBounds();
	QRect GetCornerBounds();
	QRect GetCornerBounds( int icont );
	void Copy( CPolyLine * src );
	bool TestPointInside( int x, int y );
	bool TestPointInsideContour( int icont, int x, int y );
	void AppendArc( int xi, int yi, int xf, int yf, int xc, int yc, int num );


	// access functions
	int GetNumCorners();
	int GetNumSides();
	bool GetClosed();
	int GetNumContours();
	int GetContour( int ic );
	int GetContourStart( int icont );
	int GetContourEnd( int icont );
	int GetContourSize( int icont );
	const CPolyPt & GetPt(int ic) {return corner.at(ic);}
	int GetLayer();
	int GetW();
	int GetSideStyle( int is );
	int GetSelBoxSize();
	int GetHatch(){ return m_hatch; }
	void SetX( int ic, int x );
	void SetY( int ic, int y );
	void SetEndContour( int ic, bool end_contour );
	void SetLayer( int layer );
	void SetW( int w );
	void SetSideStyle( int is, int style );
	void SetSelBoxSize( int sel_box );
	void SetHatch( int hatch ){ m_hatch = hatch; }

	// XXX TODO this needs to be moved to the CArea class or something
	int OnModified();
	// XXX move this to pad class?
	CPolyLine * MakePolylineForPad( int type, int x, int y, int w, int l, int r, int angle );

	void AddContourForPadClearance( int type, int x, int y, int w, 
						int l, int r, int angle, int fill_clearance,
						int hole_w, int hole_clearance, bool bThermal=false, int spoke_w=0 );

private:
	// GPC functions
	int MakeGpcPoly( int icontour=0, QList<CArc> * arc_array=NULL );
	int FreeGpcPoly();
	gpc_polygon * GetGpcPoly(){ return m_gpc_poly; }
	int NormalizeWithGpc( QList<CPolyLine*> * pa=NULL, bool bRetainArcs=false );
	int RestoreArcs( QList<CArc> * arc_array, QList<CPolyLine*> * pa=NULL );
	void ClipGpcPolygon( gpc_op op, CPolyLine * poly );

	/// Draws the hatch lines.
	void Hatch();

	int m_layer;	// layer to draw on
	int m_w;		// line width
	int m_sel_box;	// corner selection box width/2
	QList <CPolyPt> corner;	// array of points for corners
	QList <int> side_style;	// array of styles for sides
	int m_hatch;	// hatch style, see enum above
	int m_nhatch;	// number of hatch lines
	gpc_polygon * m_gpc_poly;	// polygon in gpc format
};
